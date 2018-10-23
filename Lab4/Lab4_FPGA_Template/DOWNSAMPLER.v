module DOWNSAMPLER
(
	data_val,
	pixel_in,
	pixel_out,
	clk
);

input data_val;
input clk;
input [15:0] pixel_in;
output [7:0] pixel_out;

reg [1:0] green;
reg [2:0] red;
reg [2:0] blue;

assign pixel_out = {red, blue, green};

always @ (posedge clk) begin
	if (data_val) begin
		red = pixel_in[14:12];
		blue = pixel_in[4:2];
		green = pixel_in[11:10];
	end
end

endmodule