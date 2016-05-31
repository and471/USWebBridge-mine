

function SnapshotController() {
	// Create offscreen canvas
	this.canvas = $("<canvas></canvas>").css("display", "none").appendTo($("body")).get(0);
}


SnapshotController.prototype.snapshot = function(videoController) {
	var width = videoController.getWidth();
	var height = videoController.getHeight();
	var video = videoController.getVideo();

	$(this.canvas).attr("width", width);
	$(this.canvas).attr("height", height);

	var context = this.canvas.getContext("2d");

	// Clear previous
	context.fillStyle = "#000";
    context.fillRect(0, 0, this.canvas.width, this.canvas.height);
    // Draw image
	context.drawImage(video, 0, 0, width, height);


	var data = this.canvas.toDataURL("image/png");
	return $("<div/>").addClass("image")
					  .data("width", width).data("height", height)
	                  .data("dataURL", data)
	                  .css("background-image", "url(" + data + ")")
	                  .data("filename", new Date().toLocaleString());
};