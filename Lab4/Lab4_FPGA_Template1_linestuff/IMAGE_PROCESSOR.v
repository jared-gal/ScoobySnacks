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
	VSYNC,
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
input VSYNC;

output [8:0] RESULT;
reg  [8:0] res;

localparam RED = 8'b111_000_00;
localparam GREEN = 8'b000_11_00;
localparam BLUE = 8'b000_001_11;
localparam [15:0] red_thresh = 16'd3000;
localparam [15:0] blue_thresh = 16'd3000;

wire [2:0] red_comp;
wire [2:0] blue_comp;
wire [1:0] green_comp;

assign red_comp = PIXEL_IN[7:5];
assign green_comp = PIXEL_IN[4:3];
assign blue_comp = PIXEL_IN[2:0];
assign RESULT = res;

reg [15:0] temp_blue = 16'b0;
reg [15:0] temp_red = 16'b0;
reg [15:0] temp_green = 16'b0;

always @ ( posedge VSYNC ) begin
	if (temp_red > temp_blue && temp_red > temp_green && temp_red > red_thresh) 
		res <= 9'd3;
	else if (temp_blue > temp_red && temp_blue > temp_green && temp_blue > blue_thresh)
		res <= 9'd2;
	else
		res <= 9'd0;
end

always @ (VGA_PIXEL_X, VGA_PIXEL_Y, VSYNC) begin
	if (VSYNC) begin
		temp_blue <= 16'b0;
		temp_red <= 16'b0;
		temp_green <= 16'b0;
	end
	else if (VGA_PIXEL_X <= 132 && VGA_PIXEL_X >= 44 && VGA_PIXEL_Y >= 36 && VGA_PIXEL_Y <= 108) begin
		if ( (blue_comp < 3'd3) && (green_comp < 2'd2) && (red_comp > 3'd5) ) begin
			temp_red <= temp_red + 1;
			temp_blue <= temp_blue;
			temp_green <= temp_green;
		end
		else if ( (red_comp < 3'd3) && (green_comp < 2'd2) && (blue_comp > 3'd5) ) begin
			temp_blue <= temp_blue + 1;
			temp_red <= temp_red;
			temp_green <= temp_green;
		end
		else if ( (green_comp > 2'd1) && (blue_comp < 3'd3) && (red_comp < 3'd3) ) begin
			temp_green <= temp_green + 1;
			temp_blue <= temp_blue;
			temp_red <= temp_red;
		end
		else begin
			temp_red <= temp_red;
			temp_blue <= temp_blue;
			temp_green <= temp_green;
		end
	end
end


endmodule