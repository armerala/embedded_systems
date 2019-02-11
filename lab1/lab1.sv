// CSEE 4840 Lab 1: Display and modify the contents of a memory
//
// Spring 2019
//
// By: <your names here>
// Uni: <your unis here>

module lab1( input logic        CLOCK_50,
	     
	     input logic [3:0] 	KEY, // Pushbuttons; KEY[0] is rightmost

	     // 7-segment LED displays; HEX0 is rightmost
	     output logic [6:0] HEX0, HEX1, HEX2, HEX3, HEX4, HEX5
	     );

   logic [3:0] 		      a;         // Address
   logic [7:0] 		      din, dout; // RAM data in and out
   logic 		      we;        // RAM write enable

   logic 		      clk;
   assign clk = CLOCK_50;

   hex7seg h0( .a(a),         .y(HEX5) ), // Leftmost digit
           h1( .a(dout[7:4]), .y(HEX3) ), // left middle
           h2( .a(dout[3:0]), .y(HEX2) ); // right middle

   controller c( .* ); // Connect everything with matching names
   memory m( .* );

   assign HEX4 = 7'b111_1111; // Display a blank; LEDs are active-low
   assign HEX1 = 7'b111_1111;
   assign HEX0 = 7'b111_1111;
  
endmodule


/**
 * Updates an internal tracker of keys pressed last cycle
 * in order to determine keys pressed down this cycle. Updates
 * the output values only on the negative clock cycle
 * in order to mitigate keybounce, double-updating, etc.
 */
module controller(input logic        clk,
		  input logic [3:0]  KEY,
		  input logic [7:0]  dout,
		  output reg [3:0] a,
		  output reg [7:0] din,
		  output reg 	     we);

	reg [3:0] prev_keys_up;
	
	//use last cycles data to figure out keys down this cycle
	always_ff @(negedge clk)
	begin

		we <= |(prev_keys_up & ~KEY);

		//key 3 down increments value
		if(~KEY[3] & prev_keys_up[3])
			a <= a - 1;

		//key 2 down increments value
		if(~KEY[2] & prev_keys_up[2])
			a <= a - 1;

		//key 1 increments val
		if(~KEY[1] & prev_keys_up[1])
			din <= dout + 1;
		
		//key 0 decrements val
		if(~KEY[0] & prev_keys_up[0])
			din <= dout - 1; 

		//keep track of last cycles signals
		prev_keys_up <= KEY;
	end
	
endmodule
		  
module hex7seg(input logic [3:0] a,
	       output logic [6:0] y);

   assign y = {a[2:0],a}; // Replace this with your code
   
endmodule

// 16 X 8 synchronous RAM with old data read-during-write behavior
module memory(input logic        clk,
	      input logic [3:0]  a,
	      input logic [7:0]  din,
	      input logic 	 we,
	      output logic [7:0] dout);
   
   logic [7:0] 			 mem [15:0];

   always_ff @(posedge clk) begin
      if (we) mem[a] <= din;
      dout <= mem[a];
   end
        
endmodule
