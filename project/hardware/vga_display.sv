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

`define VGA_WAITING_TO_RENDER 2'b00
`define VGA_INSTRUCTION_FETCH 2'b01
`define VGA_RENDERING 2'b11
`include "common.sv"


/*********************
 * MODULE STARTS HERE
 *********************/

module vga_display(
	input clk50,
	input reset,

	input render_q_lw,
    input [47:0] render_queue_dout,
	output reg render_queue_pop_front,

	input [23:0] pixel_din,
    output reg [$clog2(`SPRITE_TOTAL_PIXELS)-1:0] pixel_addr,

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
	defparam buf2.word_size = 24;
	defparam buf2.n_words = 307200;

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

	reg [7:0] vga_r;
	reg [7:0] vga_g;
	reg [7:0] vga_b;
	
	/*******************************
	 *SHOVING READ BUFFER OUT TO VGA
	 *******************************/
	logic endOfField;
	logic [10:0] hcount;
	logic [9:0] vcount;

	//state transitions on posedge
	always @(posedge clk50) begin
		state <= next_state;
		vga_r <= read_buf_dout[23:16];
		vga_g <= read_buf_dout[15:8];
		vga_b <= read_buf_dout[7:0];
	end

	//write on negedge when data is available
	always @(negedge clk50, posedge reset) begin

		if(reset) begin
            img_magic <= 0;
            render_queue_pop_front <= 1'b0;
			next_state <= `VGA_INSTRUCTION_FETCH;
		end
		else begin

            //this assign sucks but fine
            render_queue_pop_front <= (state == `VGA_INSTRUCTION_FETCH && !render_q_lw) ? 1'b1 : 1'b0;

			case(state)

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
					else if(render_queue_dout[47:0] <  9)
						next_state <= `VGA_RENDERING;
				end

				//rendering phase
				`VGA_RENDERING : begin

					if(img_load_cntr == (img_size-1)) begin
						buf1_we <= 1'b0;
						buf2_we <= 1'b0;
						next_state <= `VGA_INSTRUCTION_FETCH;
					end

					//write to b2 and destructive read on b1
					if(read_buf1) begin 
						buf2_we <= 1'b1;
						buf2_din  <= pixel_din;
						buf2_addr  <= ((img_y + img_load_cntr_y) * 640) + (img_x - img_offset_x + img_load_cntr_x);

						buf1_we <= 1'b1;
						buf1_din <= 24'h0;
						buf1_addr <= hcount[10:1] + (vcount[9:0] * 640);
					end
					//write to b1 && destructive write to b2
					else begin  
						buf1_we <= 1'b1;
						buf1_din  <= pixel_din;
						buf1_addr  <= ((img_y + img_load_cntr_y) * 640) + (img_x - img_offset_x + img_load_cntr_x);

						buf2_we <= 1'b1;
						buf2_din <= 24'h0;
						buf2_addr <= hcount[10:1] + (vcount[9:0] * 640);
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


	vga_counters counters(.clk50(clk50),.*);

	//assign final output
    assign VGA_R[7:0] = (~VGA_BLANK_n) ? 8'h01 : vga_r; 
    assign VGA_G[7:0] = (~VGA_BLANK_n) ? 8'h01 : vga_g;
    assign VGA_B[7:0] = (~VGA_BLANK_n) ? 8'hff : vga_b;
    //{read_buf_dout[23:16], read_buf_dout[15:8], read_buf_dout[7:0]};
    /*
	always_comb begin
		if (~VGA_BLANK_n)
			{VGA_R, VGA_G, VGA_B} = {8'h0, 8'h0, 8'hf};
		else
			{VGA_R, VGA_G, VGA_B} = {8'hf, 8'h0, 8'h0}; 
	end
    */
			   
endmodule
