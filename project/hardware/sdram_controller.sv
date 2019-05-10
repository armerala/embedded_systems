/******************************************************************************************
 **************************************USEFUL MACROS SECTION*******************************
 ******************************************************************************************
 * Most of these seem arbitrary, but unless indicated otherwise,
 * these definitions are arbitrary, and are required. See the datasheet
 * on the fpga sdram here: 
 *	www.intel.com/content/www/us/en/programmable/support/training/university/boards.html/
 ******************************************************************************************/

//defines for what to do during loading operation
`define ACTIVATE_STEP 2'b00
`define ISSUE_READ_STEP 2'b01
`define DATA_READY_STEP 2'b11

//state for this module
`define RESET_STATE 2'b00
`define LOADING_STATE 2'b01

`define MODE_COMMAND \ 
	mem_cs_n <= 1'b1; \
	mem_ras_n <= 1'b0; \
	mem_cas_n <= 1'b0; \
	mem_we_n <= 1'b0; \
	mem_ba <= 2'b00; \
	mem_a[10] <= 1'b0; \
	mem_a[9]  <= 1'b0; \   //burst mode
	mem_a[8:7] <= 2'b00; \
	mem_a[6:4] <= 3'b010;\ //cas latency 2
	mem_a[3] <= 1'b0;    \ //sequntial access
	mem_a[2:0] <= 3'b011;  //burst 8

`define ACTIVATE_COMMAND(bank, row_addr) \
	mem_cs_n <= 1'b0; \
	mem_ras_n <= 1'b0; \
	mem_cas_n <= 1'b1; \
	mem_we_n <= 1'b1; \
	mem_ba[1:0] <= bank; \     //bank
	mem_a[12:0] <= row_addr; \ //row-address

`define NOP_COMMAND \
	mem_cs_n <= 1'b0; \
	mem_ras_n <= 1'b1; \
	mem_cas_n <= 1'b1; \
	mem_we_n <= 1'b1;

`define READ_COMMAND(bank, start_col_addr) \
	mem_ba[1:0] <= bank; \           //bank
	mem_a[9:0] <= start_col_addr; \  //set starting col address
	mem_a[10] <= 1'b1; \             //set auto-precharge (we don't need to read more than once)

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
	input ck,                     //clk (should be same as used by sdram)
	input reset_n,                //reset signal -negative
	output reg data_available;    //amount of data available
);


parameter words_to_load = 128; //idk, however many pixels we have total
reg [31:0] img_load_counter;
reg [1:0] state;
reg [1:0] next_state;
reg [2:0] data_available; //8 bits max burst

//start cke high
initial begin
	mem_cke <= 1'b1;
	mem_ldqm <= 1'b0;
	mem_udqm <= 1'b0;
	mem_a <= 13'b0;
	mem_ba <= 2'b0;
	`NOP_COMMAND

	dq <= 16'b0;
end

/**
 * synchronize any async inputs here
 */
always @(negedge reset_n)
begin
	if(~reset_n)
		next_state <= `RESET_STATE;
end

/**
 * do work on negedge, and switch state on posedge
 * we do this because data becomes available on the posedge
 * and in general the sram operates on the posedge, thus
 * we want to issue commands and such on the negedge to be on time.
 */
always @(negedge ck)
begin

	case(state)
	begin

		//case : reset
		`RESET_STATE: begin

			`MODE_COMMAND
			img_load_counter <= 32'b0;
			data_available <= 3'b0;
			next_state <= `LOADING_STATE;
			load_op_counter <= 3'b0;
		end
		
		//case : continue load
		`LOADING_STATE: begin
			case(load_op_counter) begin
				`ACTIVATE_STEP : begin `ACTIVATE_COMMAND(1'b1,1'b1) end
				`ISSUE_READ_STEP : begin `READ_COMMAND(1'b1, 1'b1) end
				`DATA_READY_STEP : begin 
					data_available <= 3'b111; 
					`NOP_COMMAND 
				end
				default : begin `NOP_COMMAND end
			endcase
			load_op_counter <= load_op_counter + 1; //let this rollover ... conveniently, there are 4 steps, and
		end
		
	endcase

end

/**
 * state transitions and retrive data on the negedge. See above 
 * comment before negedge clock always block for reasoning on this
 */
always @(posedge ck)
begin

	state <= next_state;

	if(data_available > 3'b0) begin
		data_available <= data_available - 1'b1;
	end
end

endmodule


