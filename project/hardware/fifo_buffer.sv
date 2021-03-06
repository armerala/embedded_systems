
module fifo_buffer
	#(parameter write_word_size = 16,
	parameter read_word_size = 24,
	parameter hw_mark = 384,
	parameter lw_mark = 128,
	parameter buf_size = 512)	
	(input clk,
	input clear, //should be a syncronous input
	input we,
	input pop_front,
	input [write_word_size-1:0]  din,
	output reg [read_word_size-1:0] dout,
	output reg buf_hw, //high watermark (buffer almost full)
	output reg buf_lw  //low watermak (buffer almost empty)
);
	reg [$clog2(buf_size)-1:0] buf_begin;
	reg [$clog2(buf_size)-1:0] buf_end;
	reg [$clog2(buf_size)-1:0] buf_dif;
	reg [0:buf_size-1] mem;
	
	reg pop_front_internal;
	reg pull_pop_front_down;

initial
begin
	buf_begin <= 0;
	buf_end <= 0;
end
	
	always_ff @(posedge clk) begin

		if(clear) begin
			buf_begin <= 0;
			buf_end <= 0;
		end
		else begin

			//control pop front syncrhonously
			if(pop_front && ~pull_pop_front_down) begin
				pop_front_internal = 1'b1;
				pull_pop_front_down <= 1'b1;
			end
			else begin
				pop_front_internal = 1'b0;
				pull_pop_front_down <= 1'b0;
			end
			
			//writing
			if (we) begin
				mem[buf_end +: read_word_size] <= din;
				buf_end <= buf_end + write_word_size;
			end

			//reading/popping
			dout <= mem[buf_begin +: read_word_size];
			if(pop_front_internal) begin
				buf_begin <= buf_begin + read_word_size;
			end
			
			//end-begin dif and watermark flags
			buf_dif <= buf_end - buf_begin;
			if(buf_dif > hw_mark) begin
				buf_hw <= 1'b1;
			end
			else if(buf_dif > lw_mark) begin
				buf_hw <= 1'b0;
				buf_lw <= 1'b0;
			end
			else begin
				buf_hw <= 1'b0;
				buf_lw <= 1'b1;
			end
		end
	end

endmodule
