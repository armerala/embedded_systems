
//synchronous RAM with old data read-during-write behavior
module memory
	#(parameter word_size = 24, parameter n_words = 512)		
	(input clk,
	input we,
	input [$clog2(n_words)-1:0]  a,
	input [word_size-1:0]  din,
	output reg [word_size-1:0] dout
);
	reg [word_size-1:0] mem [n_words-1:0];

	always_ff @(posedge clk) begin
		if (we) mem[a] <= din;
		dout <= mem[a];
	end
        
endmodule
