/******************************************************************************************
 **************************************USEFUL MACROS SECTION*******************************
 ******************************************************************************************
 * Most of these seem arbitrary, but unless indicated otherwise,
 * these definitions are arbitrary, and are required. See the datasheet
 * on the fpga sdram here: 
 *	www.intel.com/content/www/us/en/programmable/support/training/university/boards.html/
 ******************************************************************************************/

//defines for what to do during loading operation

//state for this module
`define SDRAM_RESET_STATE 3'b000
`define SDRAM_ACTIVATE_STATE 3'b001
`define SDRAM_WAIT_STATE 3'b011
`define SDRAM_ISSUE_READ_STATE 3'b010
`define SDRAM_DATA_READY_STATE 3'b110

`define MODE_COMMAND \
	mem_cs_n <= 1'b1; \
	mem_ras_n <= 1'b0; \
	mem_cas_n <= 1'b0; \
	mem_we_n <= 1'b0; \
	mem_ba <= 2'b00; \
	mem_a[10] <= 1'b0; \
	/*burst_mode*/ \
	mem_a[9]  <= 1'b0; \
	mem_a[8:7] <= 2'b00; \
	/*cas latency 2*/\
	mem_a[6:4] <= 3'b010; \
	/*sequntial access*/ \
	mem_a[3] <= 1'b0; \
	/*burst 2*/ \
	mem_a[2:0] <= 3'b011

`define ACTIVATE_COMMAND(bank, row_addr) \
	mem_cs_n <= 1'b0; \
	mem_ras_n <= 1'b0; \
	mem_cas_n <= 1'b1; \
	mem_we_n <= 1'b1; \
	/*bank*/ \
	mem_ba[1:0] <= bank; \
	/*row-addr*/ \
	mem_a[12:0] <= row_addr

`define NOP_COMMAND \
	mem_cs_n <= 1'b0; \
	mem_ras_n <= 1'b1; \
	mem_cas_n <= 1'b1; \
	mem_we_n <= 1'b1

`define ISSUE_READ_COMMAND(bank, start_col_addr) \
	/*bank*/ \
	mem_ba[1:0] <= bank; \
	/*starting col addr*/ \
	mem_a[9:0] <= start_col_addr; \
	/*auto pre-charge (we dont need to read a region more than once)*/ \
	mem_a[10] <= 1'b1

/************************************************************************************************
 **************************************MODULE BEGINS HERE****************************************
 ************************************************************************************************/
module sdram_controller(

	//interface with sdram chip
	inout [15:0] mem_dq,          //sdram data line
	output reg [12:0] mem_a,      //sdram address line
	output reg [1:0] mem_ba,      //sdram mem bank line
	output reg mem_cke,           //sdram clock enable
	output reg mem_ldqm,          //sdram upper data bits mask
	output reg mem_udqm,          //sdram upper data bits mask
	output reg mem_we_n,          //sdram write enable -negative
	output reg mem_cas_n,         //sdram col addr strobe - negative
	output reg mem_ras_n,         //sdram row addr strobe - negative 
	output reg mem_cs_n,          //sdram chip select - negative
	//interface our modules
	input ck143,                  //clk (should be same as used by sdram)
	input reset_n,                //reset signal -negative
	input clear,
	input pause,                  //pause (e.g. if output buffer is full)
	input unpause,                //unpause signal
	output reg data_available     //amount of data available
);


	reg [31:0] img_load_counter;
	reg [3:0] state;
	reg [3:0] next_state;
	reg [1:0] n_data_available; //2 read burst
	reg is_paused;

	/**
	 * synchronize any async inputs here
	 */
	reg reset_n_reg;
	always @(reset_n) begin
		reset_n_reg <= reset_n;
	end

	always @(posedge pause, posedge unpause)
	begin
		if(pause)
			is_paused <= 1'b1;
		else if(unpause)
			is_paused <= 1'b0;
	end

	/**
	 * do work on negedge, and switch state on posedge
	 * we do this because data becomes available on the posedge
	 * and in general the sram operates on the posedge, thus
	 * we want to issue commands and such on the negedge to be on time.
	 */
	always @(negedge ck143, negedge reset_n_reg)
	begin
		
		if(~reset_n_reg || clear)
			next_state <= `SDRAM_RESET_STATE;
		else begin
			case(state)

				//case : reset
				`SDRAM_RESET_STATE: begin

					`MODE_COMMAND;
					mem_cke <= 1'b1;
					mem_ldqm <= 1'b0;
					mem_udqm <= 1'b0;
					img_load_counter <= 32'b0;
					n_data_available = 2'b0;
					next_state <= `SDRAM_ACTIVATE_STATE;
				end
				
				//case: issue activate command
				`SDRAM_ACTIVATE_STATE : begin 
					`ACTIVATE_COMMAND(1'b1,1'b1);
					next_state <= `SDRAM_ISSUE_READ_STATE;
				end

				//case: issue read -- only allow arrest before issuing read
				`SDRAM_ISSUE_READ_STATE : begin 
					if(~is_paused) begin
						`ISSUE_READ_COMMAND(1'b0, img_load_counter);
						img_load_counter <= img_load_counter + 1;
						next_state <= `SDRAM_WAIT_STATE;
					end
					else begin
						`NOP_COMMAND;
					end
				end
			
				//case : wait one until data is ready
				`SDRAM_WAIT_STATE : begin
					`NOP_COMMAND;
					next_state <= `SDRAM_DATA_READY_STATE;
				end

				//case: starting spitting data and raise proper flag
				`SDRAM_DATA_READY_STATE : begin 
					`NOP_COMMAND;
					n_data_available = 2'b11;
					next_state <= `SDRAM_ACTIVATE_STATE;
				end

			endcase

			data_available = (n_data_available > 2'b00) ? 1'b1 : 1'b0; 
		end

	end

	/**
	 * state transitions and retrive data on the negedge. See above 
	 * comment before negedge clock always block for reasoning on this
	 */
	always @(posedge ck143)
	begin
		state <= next_state;
	end


endmodule
