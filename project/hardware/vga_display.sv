/*
 * Avalon memory-mapped peripheral that generates VGA
 *
 * Stephen A. Edwards
 * Columbia University
 *
 *
 * 	Name / UNI
 * 		Daniel Mesko / dpm2153
 * 		Cansu Cabuk / cc4455
 *   	Alan Armero / aa3938
 */

`define VGA_RESET 2'b00
`define VGA_WAITING_TO_RENDER 2'b00
`define VGA_INSTRUCTION_FETCH 2'b00
`define VGA_RENDERING 2'b10

`define VGA_RENDER_Q_LEN 25

/*****************************
 * IMAGE LOAD DEFINES
 *****************************/
`define VGA_DO_RENDER (8'b11111111) //gets placed where image magic normally would be

//magics
`define SPRITE_MAGIC_IDLE (8'h0)
`define SPRITE_MAGIC_DUCK (8'h1)
`define SPRITE_MAGIC_PUNCH (8'h2)
`define SPRITE_MAGIC_KICK (8'h3)
`define SPRITE_MAGIC_WALK (8'h4)
`define SPRITE_MAGIC_DEAD (8'h5)
`define SPRITE_MAGIC_JUMP (8'h6)
`define SPRITE_MAGIC_POW (8'h7)
`define SPRITE_MAGIC_HEART (8'h8)

//flags
`define SPRITE_FLAG_FLIP_X (8'b 00000001)

//widths
`define SPRITE_WIDTH_IDLE 123
`define SPRITE_WIDTH_DUCK 253
`define SPRITE_WIDTH_PUNCH 329
`define SPRITE_WIDTH_KICK 282
`define SPRITE_WIDTH_WALK 170
`define SPRITE_WIDTH_DEAD 348
`define SPRITE_WIDTH_JUMP 209
`define SPRITE_WIDTH_POW 400
`define SPRITE_WIDTH_HEART 15

//heights
`define SPRITE_HEIGHT_IDLE 346
`define SPRITE_HEIGHT_DUCK 300
`define SPRITE_HEIGHT_PUNCH 340
`define SPRITE_HEIGHT_KICK 340
`define SPRITE_HEIGHT_WALK 349
`define SPRITE_HEIGHT_DEAD 109
`define SPRITE_HEIGHT_JUMP 268
`define SPRITE_HEIGHT_POW 288
`define SPRITE_HEIGHT_HEART 15

//sizes overall
`define SPRITE_SIZE_IDLE (`SPRITE_WIDTH_IDLE * `SPRITE_HEIGHT_IDLE)
`define SPRITE_SIZE_DUCK (`SPRITE_WIDTH_DUCK * `SPRITE_HEIGHT_DUCK)
`define SPRITE_SIZE_PUNCH (`SPRITE_WIDTH_PUNCH * `SPRITE_HEIGHT_PUNCH)
`define SPRITE_SIZE_KICK (`SPRITE_WIDTH_KICK * `SPRITE_HEIGHT_KICK)
`define SPRITE_SIZE_WALK (`SPRITE_WIDTH_WALK * `SPRITE_HEIGHT_WALK)
`define SPRITE_SIZE_DEAD (`SPRITE_WIDTH_DEAD * `SPRITE_HEIGHT_DEAD)
`define SPRITE_SIZE_JUMP (`SPRITE_WIDTH_JUMP * `SPRITE_HEIGHT_JUMP)
`define SPRITE_SIZE_POW (`SPRITE_WIDTH_POW * `SPRITE_HEIGHT_POW)
`define SPRITE_SIZE_HEART (`SPRITE_WIDTH_HEART * `SPRITE_HEIGHT_HEART)

`define SPRITE_TOTAL_PIXELS \
	(`SPRITE_SIZE_IDLE + \
	 `SPRITE_SIZE_DUCK + \
	`SPRITE_SIZE_PUNCH + \
	`SPRITE_SIZE_KICK + \
	`SPRITE_SIZE_WALK + \
	`SPRITE_SIZE_DEAD +	\
	`SPRITE_SIZE_JUMP + \
	`SPRITE_SIZE_POW + \
	`SPRITE_SIZE_HEART)

//mem offsets
`define SPRITE_OFFSET_IDLE (16'd 0)
`define SPRITE_OFFSET_DUCK (`SPRITE_OFFSET_IDLE + `SPRITE_SIZE_IDLE)
`define SPRITE_OFFSET_PUNCH (`SPRITE_OFFSET_DUCK + `SPRITE_SIZE_DUCK)
`define SPRITE_OFFSET_KICK (`SPRITE_OFFSET_PUNCH + `SPRITE_SIZE_PUNCH)
`define SPRITE_OFFSET_WALK (`SPRITE_OFFSET_KICK + `SPRITE_SIZE_KICK)
`define SPRITE_OFFSET_DEAD (`SPRITE_OFFSET_WALK + `SPRITE_SIZE_WALK)
`define SPRITE_OFFSET_JUMP (`SPRITE_OFFSET_DEAD + `SPRITE_SIZE_DEAD)
`define SPRITE_OFFSET_POW (`SPRITE_OFFSET_JUMP + `SPRITE_SIZE_JUMP)
`define SPRITE_OFFSET_HEART (`SPRITE_OFFSET_POW + `SPRITE_SIZE_POW)


/*********************
 * MODULE STARTS HERE
 *********************/

module vga_display(
	input clk50,
	input reset,

    input [47:0] render_queue_dout,
	output reg render_queue_pop_front,

	input [23:0] pixel_din,
    output reg [14:0] pixel_addr,

	output [7:0] VGA_R,
	output [7:0] VGA_G,
	output [7:0] VGA_B,
	output VGA_CLK, 
	output VGA_HS, 
	output VGA_VS,
	output VGA_BLANK_n,
	output VGA_SYNC_n
  );

	/**DOUBLE BUFFER THE OUTPUT**/

	//buf 1
	reg buf1_we;
	reg [$clog2(307200)-1:0] buf1_addr;
	reg [23:0] buf1_din;
	wire [23:0] buf1_dout;
	memory buf1(
		.clk(clk50),
		.we(buf1_we),
		.a(buf1_addr),
		.din(buf1_din),
		.dout(buf1_dout)
	);
	defparam buf1.word_size = 24;
	defparam buf1.n_words = 307200;

	//buf 2
	reg buf2_we;
	reg [$clog2(307200)-1:0] buf2_addr;
	reg [23:0] buf2_din;
	wire [23:0] buf2_dout;
	memory buf2(
		.clk(clk50),
		.we(buf2_we),
		.a(buf2_addr),
		.din(buf2_din),
		.dout(buf2_dout)
	);
	defparam buf1.word_size = 24;
	defparam buf1.n_words = 307200;

	//control which is read from
	reg read_buf1;
	wire [23:0] read_buf_dout;
	assign read_buf_dout = (read_buf1) ? buf1_dout : buf2_dout;

	/*******************************
     * WRITE BUFFER MODIFICATION FSM
	 ******************************/

	reg [2:0] state;
	reg [2:0] next_state;

	//keep track of currently loading sprite
	reg [7:0] img_magic;
	reg [15:0] img_x;
	reg [15:0] img_y;
	reg [7:0] img_flags;

	reg [15:0] img_size;
	reg [15:0] img_offset_x; //half width
	reg [15:0] img_offset_y; //half height
	reg [15:0] img_w;
	reg [15:0] img_h;

	//three counters to avoid doing arithmetic
	reg [15:0] img_load_cntr;   //total pixels
	reg [15:0] img_load_cntr_x; //counter on x
	reg [15:0] img_load_cntr_y; //counter on y

	//sync reset
	reg reset_reg;
	always @(reset) begin
		reset_reg <= reset;
	end

	//state transitions on posedge
	always @(posedge clk50) begin
		state <= next_state;
	end

	//write on negedge when data is available
	always @(negedge clk50, posedge reset) begin

		if(reset)
			next_state <= `VGA_RESET;
		else begin

			//I want to kill myself
			if(state != `VGA_INSTRUCTION_FETCH)
				render_queue_pop_front <= 1'b0;
			else
				render_queue_pop_front <= 1'b1;

			case(state)

				//case reset
				`VGA_RESET : begin
					img_magic <= 0;
				end

				//queue up render requests
				`VGA_WAITING_TO_RENDER : begin

					//swap buffers
					if(endOfField) begin
						read_buf1 <= ~read_buf1;
						next_state <= `VGA_INSTRUCTION_FETCH;
					end

				end
			
				//fetch next instruction
				`VGA_INSTRUCTION_FETCH : begin

					//pop instruction & decode instruction
					img_magic <= render_queue_dout[47:40];
					img_x <= render_queue_dout[39:24];
					img_y <= render_queue_dout[23:8];
					img_flags <= render_queue_dout[7:0];

					img_load_cntr <= 0;
					img_load_cntr_x <= 0;
					img_load_cntr_y <= 0;

					//mux out size and offset from magic
					case(render_queue_dout[47:40])

						`SPRITE_MAGIC_IDLE : begin
							pixel_addr <= `SPRITE_OFFSET_IDLE;
							img_size <= `SPRITE_SIZE_IDLE;
							img_w <= `SPRITE_WIDTH_IDLE;
							img_h <= `SPRITE_HEIGHT_IDLE;
							img_offset_x <= (`SPRITE_WIDTH_IDLE/2);
							img_offset_y <= (`SPRITE_HEIGHT_IDLE/2);
						end 
						`SPRITE_MAGIC_DUCK : begin
							pixel_addr <= `SPRITE_OFFSET_DUCK;
							img_size <= `SPRITE_SIZE_DUCK;
							img_w <= `SPRITE_WIDTH_DUCK;
							img_h <= `SPRITE_HEIGHT_DUCK;
							img_offset_x <= (`SPRITE_WIDTH_DUCK/2);
							img_offset_y <= (`SPRITE_HEIGHT_DUCK/2);
						end
						`SPRITE_MAGIC_PUNCH : begin
							pixel_addr <= `SPRITE_OFFSET_PUNCH;
							img_size <= `SPRITE_SIZE_PUNCH;
							img_w <= `SPRITE_WIDTH_PUNCH;
							img_h <= `SPRITE_HEIGHT_PUNCH;
							img_offset_x <= (`SPRITE_WIDTH_PUNCH/2);
							img_offset_y <= (`SPRITE_HEIGHT_PUNCH/2);
						end
						`SPRITE_MAGIC_KICK : begin
							pixel_addr <= `SPRITE_OFFSET_KICK;
							img_size <= `SPRITE_SIZE_KICK;
							img_w <= `SPRITE_WIDTH_KICK;
							img_h <= `SPRITE_HEIGHT_KICK;
							img_offset_x <= (`SPRITE_WIDTH_KICK/2);
							img_offset_y <= (`SPRITE_HEIGHT_KICK/2);
						end
						`SPRITE_MAGIC_WALK : begin
							pixel_addr <= `SPRITE_OFFSET_WALK;
							img_size <= `SPRITE_SIZE_WALK;
							img_w <= `SPRITE_WIDTH_WALK;
							img_h <= `SPRITE_HEIGHT_WALK;
							img_offset_x <= (`SPRITE_WIDTH_WALK/2);
							img_offset_y <= (`SPRITE_HEIGHT_WALK/2);
						end
						`SPRITE_MAGIC_DEAD : begin
							pixel_addr <= `SPRITE_OFFSET_DEAD;
							img_size <= `SPRITE_SIZE_DEAD;
							img_w <= `SPRITE_WIDTH_DEAD;
							img_h <= `SPRITE_HEIGHT_DEAD;
							img_offset_x <= (`SPRITE_WIDTH_DEAD/2);
							img_offset_y <= (`SPRITE_HEIGHT_DEAD/2);
						end
						`SPRITE_MAGIC_JUMP : begin
							pixel_addr <= `SPRITE_OFFSET_JUMP;
							img_size <= `SPRITE_SIZE_JUMP;
							img_w <= `SPRITE_WIDTH_JUMP;
							img_h <= `SPRITE_HEIGHT_JUMP;
							img_offset_x <= (`SPRITE_WIDTH_JUMP/2);
							img_offset_y <= (`SPRITE_HEIGHT_JUMP/2);
						end
						`SPRITE_MAGIC_POW : begin
							pixel_addr <= `SPRITE_OFFSET_POW;
							img_size <= `SPRITE_SIZE_POW;
							img_w <= `SPRITE_WIDTH_JUMP;
							img_h <= `SPRITE_HEIGHT_JUMP;
							img_offset_x <= (`SPRITE_WIDTH_JUMP/2);
							img_offset_y <= (`SPRITE_HEIGHT_JUMP/2);
						end
						`SPRITE_MAGIC_HEART : begin
							pixel_addr <= `SPRITE_OFFSET_HEART;
							img_size <= `SPRITE_SIZE_HEART;
							img_w <= `SPRITE_WIDTH_HEART;
							img_h <= `SPRITE_HEIGHT_HEART;
							img_offset_x <= (`SPRITE_WIDTH_HEART/2);
							img_offset_y <= (`SPRITE_HEIGHT_HEART/2);
						end
						
					endcase

					//figure next state
					if(render_queue_dout[47:40] == `VGA_DO_RENDER)
						next_state <= `VGA_WAITING_TO_RENDER;
					else
						next_state <= `VGA_RENDERING;
				end

				//rendering phase
				`VGA_RENDERING : begin

					if(img_load_cntr == (img_size-1)) begin
						buf1_we <= 1'b0;
						buf2_we <= 1'b0;
						next_state <= `VGA_INSTRUCTION_FETCH;
					end

					//write to b2
					if(read_buf1) begin 
						buf2_we <= 1'b1;
						buf2_din  <= pixel_din;
						buf2_addr  <= ((img_y + img_load_cntr_y) * 640) + (img_x - img_offset_x + img_load_cntr_x);
						buf1_addr <= hcount[10:1] + vcount[9:0];
					end
					//write to b1
					else begin  //write to b1
						buf1_we <= 1'b1;
						buf1_din  <= pixel_din;
						buf1_addr  <= ((img_y + img_load_cntr_y) * 640) + (img_x - img_offset_x + img_load_cntr_x);
						buf2_addr <= hcount[10:1] + vcount[9:0];
					end

					//increment load cntrs
					pixel_addr <= pixel_addr + 1;
					img_load_cntr <= img_load_cntr + 1;

					if(img_load_cntr_x == (img_w-1))
						img_load_cntr_x <= img_load_cntr_x + 1;
					else begin
						img_load_cntr_x <= 0;
						img_load_cntr_y <= img_load_cntr_y + 1;
					end
				end

			endcase //end case(state)

		end
		
	end //end always


	/*******************************
     *SHOVING READ BUFFER OUT TO VGA
	 *******************************/
	logic endOfField;
	logic [10:0] hcount;
	logic [9:0] vcount;

	vga_counters counters(.clk50(clk50),.*);

	//assign final output
	always_comb begin
		if (~VGA_BLANK_n)
			{VGA_R, VGA_G, VGA_B} = {8'h0, 8'h0, 8'hf};
		else
			{VGA_R, VGA_G, VGA_B} = {8'hf, 8'h0, 8'h0}; //{read_buf_dout[23:16], read_buf_dout[15:8], read_buf_dout[7:0]};
	end
			   
endmodule


/**********************************
 * VGA COUNTERS MODULE BEGINS HERE
 **********************************/

module vga_counters(
 input clk50, 
 input reset,
 output logic endOfField,
 output [10:0] hcount,  // hcount[10:1] is pixel column
 output [9:0]  vcount,  // vcount[9:0] is pixel row
 output VGA_CLK, 
 output VGA_HS,
 output VGA_VS, 
 output VGA_BLANK_n, 
 output VGA_SYNC_n);

/*
 * 640 X 480 VGA timing for a 50 MHz clock: one pixel every other cycle
 * 
 * HCOUNT 1599 0             1279       1599 0
 *             _______________              ________
 * ___________|    Video      |____________|  Video
 * 
 * 
 * |SYNC| BP |<-- HACTIVE -->|FP|SYNC| BP |<-- HACTIVE
 *       _______________________      _____________
 * |____|       VGA_HS          |____|
 */
   // Parameters for hcount
   parameter HACTIVE      = 11'd 1280,
             HFRONT_PORCH = 11'd 32,
             HSYNC        = 11'd 192,
             HBACK_PORCH  = 11'd 96,   
             HTOTAL       = HACTIVE + HFRONT_PORCH + HSYNC +
                            HBACK_PORCH; // 1600
   
   // Parameters for vcount
   parameter VACTIVE      = 10'd 480,
             VFRONT_PORCH = 10'd 10,
             VSYNC        = 10'd 2,
             VBACK_PORCH  = 10'd 33,
             VTOTAL       = VACTIVE + VFRONT_PORCH + VSYNC +
                            VBACK_PORCH; // 525

   logic endOfLine;
   
   always_ff @(posedge clk50 or posedge reset)
     if (reset)          hcount <= 0;
     else if (endOfLine) hcount <= 0;
     else  	         hcount <= hcount + 11'd 1;

   assign endOfLine = hcount == HTOTAL - 1;
       
   //logic endOfField;
   
   always_ff @(posedge clk50 or posedge reset)
     if (reset)          vcount <= 0;
     else if (endOfLine)
       if (endOfField)   vcount <= 0;
       else              vcount <= vcount + 10'd 1;

   assign endOfField = vcount == VTOTAL - 1;

   // Horizontal sync: from 0x520 to 0x5DF (0x57F)
   // 101 0010 0000 to 101 1101 1111
   assign VGA_HS = !( (hcount[10:8] == 3'b101) &
		      !(hcount[7:5] == 3'b111));
   assign VGA_VS = !( vcount[9:1] == (VACTIVE + VFRONT_PORCH) / 2);

   assign VGA_SYNC_n = 1'b0; // For putting sync on the green signal; unused
   
   // Horizontal active: 0 to 1279     Vertical active: 0 to 479
   // 101 0000 0000  1280	       01 1110 0000  480
   // 110 0011 1111  1599	       10 0000 1100  524
   assign VGA_BLANK_n = !( hcount[10] & (hcount[9] | hcount[8]) ) &
			!( vcount[9] | (vcount[8:5] == 4'b1111) );

   /* VGA_CLK is 25 MHz
    *             __    __    __
    * clk50    __|  |__|  |__|
    *        
    *             _____       __
    * hcount[0]__|     |_____|
    */
   assign VGA_CLK = hcount[0]; // 25 MHz clock: rising edge sensitive
   
endmodule
