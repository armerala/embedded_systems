`define QSIM_OUT_FN "./qsim.out"

module fpga_testbench;

    reg clk;
	reg reset;
    reg [7:0] hps_writedata;
    reg [2:0] hps_addr;
	reg hps_write;
	reg hps_cs;

	wire [7:0] r, g, b;
	wire hs, vs;
	wire blank_n, sync_n;
	wire vga_clk;

	fpga_top_level fpga(
		.clk50(clk),
		.reset(reset),
		.vga_clk(vga_clk),
		.vga_r(r),
		.vga_g(g),
		.vga_b(b),
		.vga_hs(hs),
		.vga_vs(vs),
		.vga_blank_n(blank_n),
		.vga_sync_n(sync_n),
		
		.hps_writedata(hps_writedata),
		.hps_write(hps_write),
		.hps_chipselect(hps_cs),
		.hps_address(hps_addr)
	);

    integer seed = 3;
    integer i;

    always
    begin
        #0.90;
        clk = ~clk;
    end


    initial
    begin

        // register setup
        clk = 0;
		reset = 1;

		@(posedge clk);
		@(posedge clk);
	
		reset = 0;

        // wait 1 clk cycle
        @(posedge clk);
        @(negedge clk);

		hps_write = 1;
		hps_cs = 1;

	
		@(negedge clk);
        for (i=0; i<90000; i=i+1)
        begin
			hps_writedata = 8'hfd;
			hps_addr = 3'h0;
            @(negedge clk);
			hps_writedata = 8'hff;
			hps_addr = 3'h1;
            @(negedge clk);
			hps_writedata = 8'hff;
			hps_addr = 3'h2;
            @(negedge clk);
			hps_writedata = 8'hff;
			hps_addr = 3'h3;
            @(negedge clk);
			hps_writedata = 0;
			hps_addr = 3'h4;
			@(negedge clk);
			hps_writedata = 0;
			hps_addr = 3'h5;
			@(negedge clk);
			hps_writedata = i;
			hps_addr = 3'h6;
			@(negedge clk);
        end

		
		@(negedge clk);
        for (i=0; i<15; i=i+1)
        begin
			hps_writedata = 8'h00;
			hps_addr = 3'h0;
            @(negedge clk);
			hps_writedata = 8'h00;
			hps_addr = 3'h1;
            @(negedge clk);
			hps_writedata = 8'h00;
			hps_addr = 3'h2;
            @(negedge clk);
			hps_writedata = 8'h00;
			hps_addr = 3'h3;
            @(negedge clk);
			hps_writedata = 8'h00;
			hps_addr = 3'h4;
			@(negedge clk);
			hps_writedata = 8'h00;
			hps_addr = 3'h5;
			@(negedge clk);
			hps_writedata = i;
			hps_addr = 3'h6;
			@(negedge clk);
        end //do a render


        for (i=0; i<22; i=i+1)
		begin
			hps_writedata = 8'hff;
			hps_addr = 3'h0;
			@(negedge clk);
		end
        for (i=0; i<90000; i=i+1)
		begin
			@(negedge clk);
		end

        $finish;
    end

endmodule
