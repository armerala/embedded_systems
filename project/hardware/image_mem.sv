
// 16 X 8 synchronous RAM with old data read-during-write behavior
module image_memory(
	input clk,
	input [3:0]  a,
	input [7:0]  din,
	input we,
	output [7:0] dout
);
	parameter word_size = 24;
	parameter n_words = $clog2(12 /*total sprite words goes here*/)-1; //TODO: need total bits across all sprites
	reg [n_words-1:0] mem [word_size-1:0];

	always_ff @(posedge clk) begin
		if (we) mem[a] <= din;
		dout <= mem[a];
	end
        
endmodule
