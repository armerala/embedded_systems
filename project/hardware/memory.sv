
//synchronous RAM with old data read-during-write behavior
module memory(
	input clk,
	input we,
	input [(mem_size_bytes/3)-1:0]  a,
	input [word_size-1:0]  din,
	output [word_size-1:0] dout
);
	parameter word_size = 24;
	parameter n_words = 512; //TODO: total words needed

	reg [n_words-1:0] mem [word_size-1:0];

	always_ff @(posedge clk) begin
		if (we) mem[a] <= din;
		dout <= mem[a];
	end
        
endmodule