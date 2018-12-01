module DOWNSAMPLER
(
	data_val,
	pixel_in,
	HREF,
	pixel_out,
	W_EN,
	clk
);

input data_val;
input clk;
input [7:0] pixel_in;
input HREF;
output [7:0] pixel_out;
output W_EN;

reg w_en;
reg [1:0] green;
reg [2:0] red;
reg [2:0] blue;

//assign pixel_out = {red, blue, green};
assign pixel_out[7:5] = blue;
assign pixel_out[4:3] = green;
assign pixel_out[2:0] = red;
assign W_EN = w_en;
reg val = 0;

always @ (posedge clk) begin
	if (!data_val && HREF) begin
		red = pixel_in[7:5];
		blue = blue;
		green = pixel_in[2:1];
		w_en = 0;
	end
	else if (data_val && HREF) begin
		if (red <= 3'd2 && pixel_in[4:2] <= 3'd2) begin
			red = 0;
			blue = 0;
			green = 0;
		end
		else if (red > 3'd4 && pixel_in[4:2] <= 3'd2) begin
			red = 3'd7;
			blue = 0;
			green = 0;
		end
		else if (pixel_in[4:2] > 3'd4 && red <= 3'd2) begin
			red = 0;
			blue = 3'd7;
			green = 0;
		end
		else begin
			red = 0;
			blue = 0;
			green = 0;
		end
		w_en = 1;
	end
	else begin
		red = 0;
		blue = 0;
		green = 0;
		w_en = 0;
	end
end

endmodule