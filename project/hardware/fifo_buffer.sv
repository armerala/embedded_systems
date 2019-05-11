
module fifo_buffer(
	input clk143,
	input we,
	input pop_front,
	input [write_word_size-1:0]  din,
	output [read_word_size-1:0] dout,
	output reg buf_hw, //high watermark (buffer almost full)
	output reg buf_lw  //low watermak (buffer almost empty)
);
	parameter write_word_size = 16;
	parameter read_word_size = 24;
	parameter buf_size = 512;

	reg [$clogs(buf_size)-1:0] buf_begin;
	reg [$clog2(buf_size)-1:0] buf_end;
	reg [$clog2(buf_size)-1:0] buf_dif;
	reg [buf_size-1:0] mem;
	
	reg pop_front_internal;

	always_ff @(negedge clk) begin
		pop_front_internal <= pop_front;
	end
	
	always_ff @(posedge clk) begin
	
		//reset signals
		pop_edge_internal <= 1'b0;

		//writing
		if (we) begin
			mem[buf_end +: write_word_size] <= din;
			buf_end <= buf_end + write_word_size;
		end

		//reading/popping
		dout <= mem[buf_begin +: read_word_size];
		if(pop_front)
			buf_begin < buf_begin + read_word_size;
		
		//end-begin dif and watermark flags
		buf_dif <= buf_end - buf_begin;
		if(buf_dif > 384) begin
			buf_hw <= 1'b1;
		end
		else if(buf_dif > 128) begin
			buf_hw <= 1'b0;
			buf_lw <= 1'b0;
		end
		else begin
			buf_hw <= 1'b0;
			buf_lw <= 1'b1;
		end
	end

);
