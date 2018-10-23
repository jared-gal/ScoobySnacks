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
input	[7:0]	PIXEL_IN;
input 		CLK;

input [9:0] VGA_PIXEL_X;
input [9:0] VGA_PIXEL_Y;
input			VGA_VSYNC_NEG;

output [8:0] RESULT;
reg  [8:0] res;

localparam RED = 8'b111_000_00;
localparam GREEN = 8'b000_111_00;
localparam BLUE = 8'b000_000_11;

wire [2:0] red_comp;
wire [2:0] blue_comp;
wire [1:0] green_comp;

assign red_comp = PIXEL_IN[7:5];
assign green_comp = PIXEL_IN[4:2];
assign blue_comp = PIXEL_IN[1:0];
assign RESULT = res;

always @ (VGA_PIXEL_X, VGA_PIXEL_Y) begin
	if ( (red_comp > blue_comp) && (red_comp > green_comp) && (red_comp >= 3'd3) ) 
		res = RED;
	else if ( (blue_comp > red_comp) && (blue_comp > green_comp) && (blue_comp >= 2'd1) )
		res = BLUE;		
	else if ( (green_comp > blue_comp) && (green_comp > red_comp) && (green_comp >= 3'd3) ) 
		res = GREEN;
	else
		res = 8'b0;
end


endmodule