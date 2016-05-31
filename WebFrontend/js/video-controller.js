function VideoController(container) {
	this.container = container;
	this.video = $("<video autoplay></video>").appendTo(this.container);
	this.canvas = $("<canvas></canvas>").addClass("overlay").appendTo(this.container);
	this.canvas.click(this.clearAlerts.bind(this));

	this.buffer = $("<canvas></canvas>");
	this.buffer_context = this.buffer.get(0).getContext("2d")

	this.context = this.canvas.get(0).getContext("2d");
	this.render();

	this.select = new RegionSelect(this.canvas);
	this.select.draw(this.draw.bind(this));
	this.select.newSelection(this.onNewSelection.bind(this));
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
	this.container.css("transform", "scale(" + zoom + ")");
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

VideoController.prototype.enableCrop = function() {
	this.select.enable();

	// Copy the current contents of the video outside the selected region into the overlay canvas
	var width = this.getWidth();
	var height = this.getHeight();
	var video = this.getVideo();

	// Fix container and canvas dimensions while cropping
	this.container.css("width",  width).css("height", height);
	this.canvas.attr("width", width).attr("height", height);

	// Save current video content
	this.buffer.attr("width", width).attr("height", height);
	this.buffer_context.clearRect(0, 0, width, height);
	this.buffer_context.drawImage(video, 0, 0, width, height);

	this.draw();
}

VideoController.prototype.draw = function() {
	// Draw video
	this.context.drawImage(this.buffer.get(0), 0, 0, this.buffer.attr("width"), this.buffer.attr("height"));
}

VideoController.prototype.onNewSelection = function(selection) {
	this.cropCB(selection);

	this.video.css({
		position: "absolute", 
		top: selection.top(), 
		left: selection.left(), 
	});
}

VideoController.prototype.getVideo = function() { return this.video.get(0) }
// Always up to date
VideoController.prototype.getHeight = function() { return this.video.get(0).videoHeight }
VideoController.prototype.getWidth = function() { return this.video.get(0).videoWidth }

VideoController.prototype.crop = function(cb) {
	this.cropCB = cb;
}