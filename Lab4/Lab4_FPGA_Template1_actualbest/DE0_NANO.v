`define SCREEN_WIDTH 176
`define SCREEN_HEIGHT 144

///////* DON'T CHANGE THIS PART *///////
module DE0_NANO(
	CLOCK_50,
	GPIO_0_D,
	GPIO_1_D,
	KEY
);

//=======================================================
//  PARAMETER declarations
//=======================================================
localparam RED = 8'b111_000_00;
localparam GREEN = 8'b000_111_00; // WE SWITCHED GREEN AND BLUE AHHHHHH
localparam BLUE = 8'b000_000_11;

//=======================================================
//  PORT declarations
//=======================================================

//////////// CLOCK - DON'T NEED TO CHANGE THIS //////////
input 		          		CLOCK_50;

//////////// GPIO_0, GPIO_0 connect to GPIO Default //////////
output 		    [33:0]		GPIO_0_D;
//////////// GPIO_0, GPIO_1 connect to GPIO Default //////////
input           [10:24]		GPIO_1_D;
input 		     [1:0]		KEY;

///// PIXEL DATA /////
wire [7:0]	pixel_data_RGB332; // this used to be a reg

///// READ/WRITE ADDRESS /////
reg [14:0] X_ADDR;
reg [14:0] Y_ADDR;
wire [14:0] WRITE_ADDRESS;
reg [14:0] READ_ADDRESS; 

assign WRITE_ADDRESS = X_ADDR + Y_ADDR*(`SCREEN_WIDTH);

///// VGA INPUTS/OUTPUTS /////
wire 			VGA_RESET;
wire [7:0]	VGA_COLOR_IN;
wire [9:0]	VGA_PIXEL_X;
wire [9:0]	VGA_PIXEL_Y;
wire [7:0]	MEM_OUTPUT;
wire			VGA_VSYNC_NEG;
wire			VGA_HSYNC_NEG;
reg			VGA_READ_MEM_EN;

assign GPIO_0_D[5] = VGA_VSYNC_NEG;
assign VGA_RESET = ~KEY[0];

///// I/O for Img Proc /////
wire [8:0] RESULT;

/* WRITE ENABLE */
wire W_EN;

///////* CREATE ANY LOCAL WIRES YOU NEED FOR YOUR PLL *///////
wire c0_sig;
wire c1_sig;
wire c2_sig;

///////* INSTANTIATE YOUR PLL HERE *///////
PLL	PLL_inst (
	.inclk0 ( CLOCK_50 ),
	.c0 ( c0_sig ), //24 Mhz
	.c1 ( c1_sig ), //25 Mhz
	.c2 ( c2_sig )  //50 Mhz
);

///////* M9K Module *///////
Dual_Port_RAM_M9K mem(
	.input_data(pixel_data_RGB332),
	.w_addr(WRITE_ADDRESS),
	.r_addr(READ_ADDRESS),
	.w_en(W_EN),
	.clk_W(c2_sig),
	.clk_R(c1_sig), // DO WE NEED TO READ SLOWER THAN WRITE??
	.output_data(MEM_OUTPUT)
);

///////* VGA Module *///////
VGA_DRIVER driver (
	.RESET(VGA_RESET),
	.CLOCK(c1_sig),
	.PIXEL_COLOR_IN(VGA_READ_MEM_EN ? MEM_OUTPUT : BLUE),
	.PIXEL_X(VGA_PIXEL_X),
	.PIXEL_Y(VGA_PIXEL_Y),
	.PIXEL_COLOR_OUT({GPIO_0_D[9],GPIO_0_D[11],GPIO_0_D[13],GPIO_0_D[15],GPIO_0_D[17],GPIO_0_D[19],GPIO_0_D[21],GPIO_0_D[23]}),
    .H_SYNC_NEG(GPIO_0_D[7]),
    .V_SYNC_NEG(VGA_VSYNC_NEG)
);

///////* Image Processor *///////
IMAGE_PROCESSOR proc(
	.PIXEL_IN(MEM_OUTPUT),
	.CLK(c1_sig),
	.VGA_PIXEL_X(VGA_PIXEL_X),
	.VGA_PIXEL_Y(VGA_PIXEL_Y),
	.VGA_VSYNC_NEG(VGA_VSYNC_NEG),
	.RESULT(RESULT)
);

//////* DOWNSAMPLER *///////////
DOWNSAMPLER down (
	.data_val(data_valid),
	.clk(PCLK),
	.pixel_in(raw_data),
	.pixel_out(pixel_data_RGB332),
	.W_EN(W_EN)
);

assign GPIO_0_D[1] = c0_sig; // output 24 MHz clock to camera

reg [15:0] raw_data;
reg data_valid = 0;
reg first = 0;
reg [7:0] pixel_y = 0;
reg [7:0] pixel_x = 0;
wire D7, D6, D5, D4, D3, D2, D1, D0;
wire HREF, PCLK, VSYNC;
assign D7 = GPIO_1_D[14];
assign D6 = GPIO_1_D[15];
assign D5 = GPIO_1_D[16];
assign D4 = GPIO_1_D[17];
assign D3 = GPIO_1_D[18];
assign D2 = GPIO_1_D[19];
assign D1 = GPIO_1_D[20];
assign D0 = GPIO_1_D[21];
assign HREF = GPIO_1_D[11];
assign PCLK = GPIO_1_D[12];
assign VSYNC = GPIO_1_D[10];

always @ (negedge HREF) begin
	if (VSYNC)
		Y_ADDR <= 0;
	if (Y_ADDR >= `SCREEN_HEIGHT - 1) begin
		Y_ADDR <= 0;
	end
	else begin 
		Y_ADDR <= Y_ADDR + 1'd1;
	end
end

always @ (posedge PCLK) begin
	if (VSYNC) begin
		X_ADDR <= 0;
		data_valid <= 1'd0;
		first <= 0;
		//W_EN = 0;
	end
	else if (!first && HREF && !VSYNC) begin
		//W_EN <= 0;
		raw_data[8] <= D0;
		raw_data[9] <= D1;
		raw_data[10] <= D2;
		raw_data[11] <= D3;
		raw_data[12] <= D4;
		raw_data[13] <= D5;
		raw_data[14] <= D6;
		raw_data[15] <= D7;
		data_valid <= 1'd0;
		first <= 1'd1;
		X_ADDR <= X_ADDR;
	end
	else if (first && HREF && !VSYNC) begin
		//W_EN <= 1;
		//raw_data[15:8] = {D7, D6, D5, D4, D3, D2, D1, D0};
		raw_data[0] <= D0;
		raw_data[1] <= D1;
		raw_data[2] <= D2;
		raw_data[3] <= D3;
		raw_data[4] <= D4;
		raw_data[5] <= D5;
		raw_data[6] <= D6;
		raw_data[7] <= D7;
		data_valid <= 1'd1;
		first <= 1'd0;
		X_ADDR <= X_ADDR + 1'd1;
	end
	else begin
		//W_EN = 0;
		X_ADDR <= 0;
		data_valid <= 1'd0;
		first <= 0;
	end
		
end	

///////* Update Read Address *///////
always @ (VGA_PIXEL_X, VGA_PIXEL_Y) begin
		READ_ADDRESS = (VGA_PIXEL_X + VGA_PIXEL_Y*`SCREEN_WIDTH);
		if(VGA_PIXEL_X>(`SCREEN_WIDTH-1) || VGA_PIXEL_Y>(`SCREEN_HEIGHT-1))begin
				VGA_READ_MEM_EN = 1'b0;
		end
		else begin
			VGA_READ_MEM_EN = 1'b1;
		end
end

///////* Display Test Pattern *///////
/*reg [2:0] red;
reg [1:0] blue;
reg [2:0] green;
wire [7:0] color;
assign color[7:5] = red;
assign color[4:2] = blue;
assign color[1:0] = green;
assign pixel_data_RGB332 = color;*/

///////* Update Write Address *///////
/*always @ (VGA_PIXEL_X, VGA_PIXEL_Y) begin
		WRITE_ADDRESS = (VGA_PIXEL_X + VGA_PIXEL_Y*`SCREEN_WIDTH);
		if(VGA_PIXEL_X>(`SCREEN_WIDTH-1) || VGA_PIXEL_Y>(`SCREEN_HEIGHT-1))begin
				W_EN = 1'b0;
		end
		else begin
				W_EN = 1'b1;
		end
		
		if (VGA_PIXEL_X < 10'd22) begin
			red = 0;
			blue = 0;
			green = 0;
		end
		else if (VGA_PIXEL_X < 10'd44 && VGA_PIXEL_Y < 10'd48) begin
			red = 1;
			green = 0;
			blue = 0;
		end
		else if (VGA_PIXEL_X < 10'd66 && VGA_PIXEL_Y < 10'd48) begin
			red = 2;
			green = 0;
			blue = 0;
		end
		else if (VGA_PIXEL_X < 10'd88 && VGA_PIXEL_Y < 10'd48) begin
			red = 3;
			green = 0;
			blue = 0;
		end
		else if (VGA_PIXEL_X < 10'd110 && VGA_PIXEL_Y < 10'd48) begin
			red = 4;
			green = 0;
			blue = 0;
		end
		else if (VGA_PIXEL_X < 10'd132 && VGA_PIXEL_Y < 10'd48) begin
			red = 5;
			green = 0;
			blue = 0;
		end
		else if (VGA_PIXEL_X < 10'd154 && VGA_PIXEL_Y < 10'd48) begin
			red = 6;
			green = 0;
			blue = 0;
		end
		else if (VGA_PIXEL_X < 10'd176 && VGA_PIXEL_Y < 10'd48) begin
			red = 7;
			green = 0;
			blue = 0;
		end
		else if (VGA_PIXEL_X < 10'd44 && VGA_PIXEL_Y < 10'd96) begin
			red = 1;
			green = 1;
			blue = 0;
		end
		else if (VGA_PIXEL_X < 10'd66 && VGA_PIXEL_Y < 10'd96) begin
			red = 2;
			green = 2;
			blue = 0;
		end
		else if (VGA_PIXEL_X < 10'd88 && VGA_PIXEL_Y < 10'd96) begin
			red = 3;
			green = 3;
			blue = 0;
		end
		else if (VGA_PIXEL_X < 10'd110 && VGA_PIXEL_Y < 10'd96) begin
			red = 4;
			green = 4;
			blue = 0;
		end
		else if (VGA_PIXEL_X < 10'd132 && VGA_PIXEL_Y < 10'd96) begin
			red = 5;
			green = 5;
			blue = 0;
		end
		else if (VGA_PIXEL_X < 10'd154 && VGA_PIXEL_Y < 10'd96) begin
			red = 6;
			green = 6;
			blue = 0;
		end
		else if (VGA_PIXEL_X < 10'd176 && VGA_PIXEL_Y < 10'd96) begin
			red = 7;
			green = 7;
			blue = 0;
		end
		else if (VGA_PIXEL_X < 10'd44 && VGA_PIXEL_Y < 10'd148) begin
			red = 1;
			green = 1;
			blue = 1;
		end
		else if (VGA_PIXEL_X < 10'd66 && VGA_PIXEL_Y < 10'd148) begin
			red = 2;
			green = 2;
			blue = 1;
		end
		else if (VGA_PIXEL_X < 10'd88 && VGA_PIXEL_Y < 10'd148) begin
			red = 3;
			green = 3;
			blue = 2;
		end
		else if (VGA_PIXEL_X < 10'd110 && VGA_PIXEL_Y < 10'd148) begin
			red = 4;
			green = 4;
			blue = 2;
		end
		else if (VGA_PIXEL_X < 10'd132 && VGA_PIXEL_Y < 10'd148) begin
			red = 5;
			green = 5;
			blue = 3;
		end
		else if (VGA_PIXEL_X < 10'd154 && VGA_PIXEL_Y < 10'd148) begin
			red = 6;
			green = 6;
			blue = 3;
		end
		else if (VGA_PIXEL_X < 10'd176 && VGA_PIXEL_Y < 10'd148) begin
			red = 7;
			green = 7;
			blue = 3;
		end
		else begin
			red = red;
			green = green;
			blue = blue;
		end
end*/

	
endmodule 