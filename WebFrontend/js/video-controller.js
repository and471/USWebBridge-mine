function VideoController(container) {
	this.container = container;
	this.video = $("<video autoplay></video>").appendTo(this.container);
	this.canvas = $("<canvas></canvas>").addClass("overlay").appendTo(this.container);
	this.canvas.click(this.clearAlerts.bind(this));

	this.context = this.canvas.get(0).getContext("2d");
	this.render();
}

VideoController.prototype.pause = function() {
	this.video.get(0).pause();
};

VideoController.prototype.play = function() {
	this.video.get(0).play();
};

VideoController.prototype.onRemoteStream = function(stream) {
	attachMediaStream(this.video.get(0), stream);
}

VideoController.prototype.zoom = function(zoom) {
	this.canvas.css("transform", "scale(" + zoom + ")");
	this.video.css("transform", "scale(" + zoom + ")");
}

VideoController.prototype.render = function() {
    requestAnimationFrame(this.render.bind(this));
}

VideoController.prototype.clearAlerts = function(content, duration) {
	this.container.find(".alert").fadeOut();
}

VideoController.prototype.showAlert = function(content, duration) {
	duration = duration ? duration : 1000;

	var alert = $("<div></div>").addClass("alert alert-info")
								.text(content)
								.appendTo(this.container);

	setTimeout(function() {
		alert.fadeOut();
	}.bind(this), duration);
}

VideoController.prototype.crop = function(x1, x2, y1, y2) {
	if (x1 == x2 == y1 == y2 == -1) {
		// Clear overlay canvas

		return;
	}

	// Copy the current contents of the video outside the selected region into the overlay canvas
	var width = this.getWidth();
	var height = this.getHeight();
	var video = this.getVideo();

	$(this.canvas).attr("width", width);
	$(this.canvas).attr("height", height);

    // Draw video
	this.context.drawImage(video, 0, 0, width, height);
	
	// Clear cropped region
	this.context.save();
	this.context.globalCompositeOperation = 'destination-out';
	this.context.fillStyle = "white";
    this.context.fillRect(x1, y1, x2-x1, y2-y1);
    this.context.restore();
}

VideoController.prototype.getVideo = function() { return this.video.get(0) }
// Always up to date
VideoController.prototype.getHeight = function() { return this.video.get(0).videoHeight }
VideoController.prototype.getWidth = function() { return this.video.get(0).videoWidth }
