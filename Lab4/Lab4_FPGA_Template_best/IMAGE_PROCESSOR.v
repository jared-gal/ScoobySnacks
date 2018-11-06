`define SCREEN_WIDTH 176
`define SCREEN_HEIGHT 144
`define NUM_BARS 3
`define BAR_HEIGHT 48

module IMAGE_PROCESSOR (
	PIXEL_IN,
	CLK,
	VGA_PIXEL_X,
	VGA_PIXEL_Y,
	VGA_VSYNC_NEG,
	RESULT
);


//=======================================================
//  PORT declarations
//=======================================================
input	[7:0]	PIXEL_IN; // R3 G3 B2
input 		CLK;

input [9:0] VGA_PIXEL_X;
input [9:0] VGA_PIXEL_Y;
input			VGA_VSYNC_NEG;

output [8:0] RESULT;

//=======================================================
// Registers
//=======================================================
reg red  = 1'd0;
reg blue = 1'd0;

reg [8:0] res;
assign RESULT = res;

always @ ( posedge CLK ) begin
	// If we've reached the end of a frame, compare red and blue, and output accordingly.
	// For now, say red = RESULT[1] high, and blue = RESULT[0] high
	if ( VGA_PIXEL_X == 0 && VGA_PIXEL_Y == 0 ) begin
		// compare red and blue
		if ( red > blue ) begin
			res[8:0] = 9'b10;
		end
		
		else if ( blue > red ) begin
			res[8:0] = 9'b01;
		end
		
		else begin
			res[8:0] = 9'b0;	// neither maj red nor maj blue
		end
	end
	
	// Otherwise, increment red and blue
	else begin
		// if at least 2 of 3 red bits are high, red += 1
		if ( (PIXEL_IN[7] && PIXEL_IN[6]) || (PIXEL_IN[7] && PIXEL_IN[5]) || (PIXEL_IN[5] && PIXEL_IN[6]) ) begin
			red = red + 1;
		end
		
		// if both blue bits are high, blue += 1
		if ( (PIXEL_IN[1] && PIXEL_IN[0]) ) begin
			blue = blue + 1;
		end
	end

end



endmodule
