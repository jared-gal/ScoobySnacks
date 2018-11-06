`define SCREEN_WIDTH 176
`define SCREEN_HEIGHT 144

///////* DON'T CHANGE THIS PART *///////
module DE0_NANO(
	CLOCK_50,
	GPIO_0_D,
	GPIO_1_D,
	KEY,
	LED
	
);

output [7:0] LED;
reg [7:0] led;

assign LED = led;
//=======================================================
//  PARAMETER declarations
//=======================================================
localparam RED = 8'b111_000_00;
localparam GREEN = 8'b000_111_00;
localparam BLUE = 8'b000_000_11;

//=======================================================
//  PORT declarations
//=======================================================

//////////// CLOCK - DON'T NEED TO CHANGE THIS //////////
input 		          		CLOCK_50;

//////////// GPIO_0, GPIO_0 connect to GPIO Default //////////
output 		    [33:0]		GPIO_0_D;
//////////// GPIO_0, GPIO_1 connect to GPIO Default //////////
input 		    [33:0]		GPIO_1_D;
input 		     [1:0]		KEY;

///// PIXEL DATA /////
reg [7:0]	pixel_data_RGB332 = 8'd0;
/////CAMERA INPUT/////
wire HREF = GPIO_1_D[10];
wire [7:0] CameraInput = GPIO_1_D[19:12];
wire V_SYNC = GPIO_1_D[11];
/////REG for DOWNSAMLEPER/////
reg i= 1'd0;

///// READ/WRITE ADDRESS /////
reg [14:0] X_ADDR;
reg [14:0] Y_ADDR;
wire [14:0] WRITE_ADDRESS;
reg [14:0] READ_ADDRESS; 

assign WRITE_ADDRESS = (X_ADDR + Y_ADDR*(`SCREEN_WIDTH));

///// VGA INPUTS/OUTPUTS /////
reg 			VGA_RESET;
wire [7:0]	VGA_COLOR_IN;
wire [9:0]	VGA_PIXEL_X;
wire [9:0]	VGA_PIXEL_Y;
wire [7:0]	MEM_OUTPUT;
wire			VGA_VSYNC_NEG;
wire			VGA_HSYNC_NEG;
reg			VGA_READ_MEM_EN;

assign GPIO_0_D[5] = VGA_VSYNC_NEG;
//assign VGA_RESET = ~KEY[0];

///// I/O for Img Proc /////
wire [8:0] RESULT;

/* WRITE ENABLE */
reg W_EN;

///////* CREATE ANY LOCAL WIRES YOU NEED FOR YOUR PLL *///////

wire clock0; 
wire clock1;
wire clock2;


///////* INSTANTIATE YOUR PLL HERE *///////

RobotHell	RobotHell_inst (
	.inclk0 ( CLOCK_50 ),
	.c0 ( clock0 ), //24 MHZ
	.c1 ( clock1 ), //25 MHZ for driving VGA
	.c2 ( clock2 )  //50 MHZ
	);
	
assign GPIO_0_D[33] = clock0;

///////* M9K Module *///////
Dual_Port_RAM_M9K mem(
	.input_data(pixel_data_RGB332),
	.w_addr(WRITE_ADDRESS),
	.r_addr(READ_ADDRESS),
	.w_en(W_EN),
	.clk_W(clock2),
	.clk_R(clock1), 
	.output_data(MEM_OUTPUT)
);

///////* VGA Module *///////
VGA_DRIVER driver (
	.RESET(VGA_RESET),
	.CLOCK(clock1),
	.PIXEL_COLOR_IN(VGA_READ_MEM_EN ? MEM_OUTPUT : RED), //changed blue to VGA_COLOR_IN
	.PIXEL_X(VGA_PIXEL_X),
	.PIXEL_Y(VGA_PIXEL_Y),
	.PIXEL_COLOR_OUT({GPIO_0_D[9],GPIO_0_D[11],GPIO_0_D[13],GPIO_0_D[15],GPIO_0_D[17],GPIO_0_D[19],GPIO_0_D[21],GPIO_0_D[23]}),
   .H_SYNC_NEG(GPIO_0_D[7]),
   .V_SYNC_NEG(VGA_VSYNC_NEG)
);

///////* Image Processor *///////
IMAGE_PROCESSOR proc(
	.PIXEL_IN(MEM_OUTPUT),
	.CLK(clock1),
	.VGA_PIXEL_X(VGA_PIXEL_X),
	.VGA_PIXEL_Y(VGA_PIXEL_Y),
	.VGA_VSYNC_NEG(VGA_VSYNC_NEG),
	.RESULT(RESULT)
);

///////* Update Read Address *///////
always @ (VGA_PIXEL_X, VGA_PIXEL_Y) begin
		READ_ADDRESS = (VGA_PIXEL_X + VGA_PIXEL_Y*`SCREEN_WIDTH);
		if(VGA_PIXEL_X>(`SCREEN_WIDTH-1) || VGA_PIXEL_Y>(`SCREEN_HEIGHT-1))begin
				VGA_READ_MEM_EN = 1'b0; //Red
		end
		else begin
				VGA_READ_MEM_EN = 1'b1; //Black Square (Reads from memory in this area)
		end
end

//Write a test pattern to memory
//always @ ( posedge clock1 ) begin
//		// First, write.
//		W_EN = 1;
//		if( (X_ADDR > 15'd5 && X_ADDR < 15'd90) || (Y_ADDR > 15'd5 && Y_ADDR < 15'd90))begin
//			pixel_data_RGB332 = BLUE;
//		end
//		else begin
//			pixel_data_RGB332 = GREEN;
//		end
//		
//		// Now, update x and y
//		if (X_ADDR < `SCREEN_WIDTH)begin
//			X_ADDR = X_ADDR + 15'd1;
//		end
//		else begin
//			X_ADDR = 0;
//		end
//		
//		if (Y_ADDR < `SCREEN_HEIGHT)begin
//			Y_ADDR = Y_ADDR + 15'd1;
//		end
//		else begin
//			Y_ADDR = 0;
//		end
//end


//Down-sampler
always @ ( posedge clock0 ) begin
	VGA_RESET = V_SYNC;
	if (VGA_RESET == 1)begin // V_SYNC marks the end of a frame: reset X and Y.
		W_EN   <= 1'b0;			// Don't write to memory!
		X_ADDR <= 15'd0; // Beginning of next row
		Y_ADDR <= 15'd0; // Move down to next row
	end
	else begin
		if (HREF == 1)begin
			if (i == 0)begin 	// First byte received of two.
				W_EN <= 1'b0;  // DON'T WRITE TO MEMORY YET!
				pixel_data_RGB332[7] <= CameraInput[7]; // Red
				pixel_data_RGB332[6] <= CameraInput[6]; // Red 
				pixel_data_RGB332[5] <= CameraInput[5]; // Red
				pixel_data_RGB332[4] <= CameraInput[2]; // Green
				pixel_data_RGB332[3] <= CameraInput[1]; // Green
				pixel_data_RGB332[2] <= CameraInput[0]; // Green
				i <= 1;
			end
			
			else 					// i = 1 : second byte!
			begin
				pixel_data_RGB332[1] <= CameraInput[4]; // BLUE
				pixel_data_RGB332[0] <= CameraInput[3]; // Blue
				// Now the pixel is finished!  Update WRITE_ADDRESS and enable memory writing
				W_EN <= 1'b1;	// Memory is written.
				X_ADDR <= X_ADDR + 15'd1; // Update X AFTER memory is written.
				i <= 0;
			end

		end

		else // HREF is LOW, i.e. end of a row
		begin
			W_EN   <= 1'b0;			// Don't write anything here!!
			X_ADDR <= 15'd0; 						// Beginning of next row
			Y_ADDR <= Y_ADDR + 15'd1;	// Move down to next row
		end
	end
	

end



//test to see if image processor works
//always @ (clock1) begin
//if(RESULT == 9'b10) //RED
//	begin
//		led = 8'b00000010;
//	end
//else if (RESULT == 9'b01) //BLUE
//	begin
//		led = 8'b00000001;
//	end
//else
//	begin
//		led = 8'b00000000;
//	end
//
//
//end
//
endmodule
