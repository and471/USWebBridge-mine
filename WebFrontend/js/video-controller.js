function VideoController(container) {
	this.container = container;
	this.video = $("<video autoplay></video>").appendTo(this.container);
	this.canvas = $("<canvas></canvas>").addClass("overlay").appendTo(this.container);
	this.canvas.click(this.clearAlerts.bind(this));

	this.buffer = $("<canvas></canvas>");
	this.buffer_context = this.buffer.get(0).getContext("2d");
	this.initialSelection = true;

	this.context = this.canvas.get(0).getContext("2d");
	this.render();

	this.select = new RegionSelect(this.canvas);
	this.select.requestDraw(this.draw.bind(this));
	this.select.newSelection(this.onNewSelection.bind(this));

    // Prevent selecting
 	var preventHighlighting = function(e) { e.preventDefault(); return false; };
    this.canvas.bind('selectstart', preventHighlighting);
    this.container.bind('selectstart', preventHighlighting);
    this.container.parent().bind('selectstart', preventHighlighting);
    this.video.bind('selectstart', preventHighlighting);

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
	var fadeOut = 400;

	var alert = $("<div></div>").addClass("alert alert-orange")
								.text(content)
								.appendTo(this.container);

	setTimeout(function() {
		alert.fadeOut(fadeOut);
	}.bind(this), duration);

	setTimeout(function() {
		alert.remove();
	}.bind(this), duration+fadeOut*2);
}

VideoController.prototype.enableCrop = function() {
	this.select.enable();
	this.canvas.show();

	// Copy the current contents of the video outside the selected region into the overlay canvas
	var width = this.getWidth();
	var height = this.getHeight();
	var video = this.getVideo();

	// Fix container and canvas dimensions while cropping
	this.container.css({width: width, height: height, display: "block"});
	this.canvas.attr("width", width).attr("height", height);

	// Save current video content
	this.buffer.attr("width", width).attr("height", height);
	this.buffer_context.clearRect(0, 0, width, height);
	this.buffer_context.drawImage(video, 0, 0, width, height);
	// Apply blur
	StackBlur.canvasRGB(this.buffer.get(0), 0, 0, width, height, 15);

	//this.draw();
	this.initialSelection = true;
}

VideoController.prototype.disableCrop = function() {
	this.drawBuffer(this.select.getSelection().left(),  this.select.getSelection().top());

	this.select.disableAndClear();

	var resetFunction = function() {
		this.container.css({width: "auto", height: "auto", display: "table"});
		this.video.css({position: "static"});
		this.canvas.hide();
		this.draw();
	}.bind(this);

	if (this.initialSelection) {
		resetFunction();
	} else {
		new VideoSizeListener(this.video, resetFunction);
	}
	
	this.cropCB(Selection.reset());
	this.initialSelection = false;
}

VideoController.prototype.draw = function() {
	// Draw blurred video
	this.context.drawImage(this.buffer.get(0), 0, 0, this.buffer.attr("width"), this.buffer.attr("height"));

	// Draw selection border + clear selected region
	this.select.draw(this.context);
}

VideoController.prototype.drawBuffer = function(left, top) {
	// Draw the current video frame in the selection while waiting for stream to switch
	this.context.save()
	this.context.globalCompositeOperation = 'destination-over';
	this.context.drawImage(this.video.get(0), left, top, this.getWidth(), this.getHeight());
	this.context.restore();
}

VideoController.prototype.onNewSelection = function(selection) {
	if (selection.isReset()) {
		this.drawBuffer(this.select.getSelection().left(), this.select.getSelection().top());

		if (!this.initialSelection) {
			new VideoSizeListener(this.video, function() {
				this.video.css({position: "static"});
				this.draw();
			}.bind(this));
		}
		this.cropCB(selection);
	} else {
		// Instead of showing the video in the selected region, 
		// show the current contents of the video, until the stream switches
		this.draw();
		this.drawBuffer(0, 0);

		new VideoSizeListener(this.video, function(selection) {
			this.video.css({
				position: "absolute", 
				top: selection.top(), 
				left: selection.left(), 
			});
			this.draw();
		}.bind(this, selection));
		this.cropCB(selection);
	}

	this.initialSelection = false;
}

VideoController.prototype.getVideo = function() { return this.video.get(0) }
// Always up to date
VideoController.prototype.getHeight = function() { return this.video.get(0).videoHeight }
VideoController.prototype.getWidth = function() { return this.video.get(0).videoWidth }

VideoController.prototype.crop = function(cb) {
	this.cropCB = cb;
}

function VideoSizeListener(video, callback) {
	this.video = video.get(0);
	this.width = this.video.videoWidth;
	this.height = this.video.videoHeight;
	this.callback = callback;

	this.listen();
}

VideoSizeListener.prototype.listen = function() {
	if (this.width != this.video.videoWidth || this.height != this.video.videoHeight) {
		this.callback();
	} else {
		setTimeout(this.listen.bind(this), 100);
	}
}