function VideoController(container,  wrapper, toggleCalip) {
	this.container = container;
	this.wrapper = wrapper;
	this.video = $("<video autoplay></video>").appendTo(this.container);
	this.regionCanvas = $("<canvas></canvas>").addClass("overlay region").appendTo(this.container).hide();
	this.regionCanvas.click(this.clearAlerts.bind(this));

	this.caliperCanvas = $("<canvas></canvas>").addClass("overlay calipers").appendTo(this.container).hide();

	this.buffer = $("<canvas></canvas>");
	this.buffer_context = this.buffer.get(0).getContext("2d");
	this.initialSelection = true;

	this.regionContext = this.regionCanvas.get(0).getContext("2d");
	this.caliperContext = this.caliperCanvas.get(0).getContext("2d");

	this.select = new RegionSelect(this.regionCanvas);
	this.select.requestDraw(this.regionDraw.bind(this));
	this.select.newSelection(this.onNewSelection.bind(this));

	this.calipers = new Calipers(this.caliperCanvas);
	this.calipers.lineChanged(this.onLineChanged.bind(this));
	this.length = null;

    // Prevent selecting
 	var preventHighlighting = function(e) { e.preventDefault(); return false; };
    this.regionCanvas.bind('selectstart', preventHighlighting);
    this.caliperCanvas.bind('selectstart', preventHighlighting);
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

VideoController.prototype.clearAlerts = function(content, duration) {
	this.container.find(".alert").fadeOut();
}

VideoController.prototype.showAlert = function(content, duration) {
	duration = duration ? duration : 1000;
	var fadeOut = 400;

	var alert = $("<div></div>").addClass("alert alert-orange")
								.text(content)
								.appendTo(this.wrapper);

	setTimeout(function() {
		alert.fadeOut(fadeOut);
	}.bind(this), duration);

	setTimeout(function() {
		alert.remove();
	}.bind(this), duration+fadeOut*2);
}

VideoController.prototype.updateLength = function(length) {
	if (!this.length) {
		this.length = $("<div></div>").addClass("alert alert-dark length")
									  .appendTo(this.wrapper);
	}
	length *= this.pixelSize;

	// Round to 3dp
	length = length.toFixed(3);
	this.length.text(length + "mm");
}


VideoController.prototype.enableCrop = function() {
	if (this.calipers.enabled) {
		this.toggleCalipers.onToggle();
	}

	this.resizeCanvas(this.regionCanvas);

	this.select.enable();
	this.regionCanvas.show();

	var width = this.getWidth();
	var height = this.getHeight();

	// Save current video content
	this.buffer.attr("width", width).attr("height", height);
	this.buffer_context.clearRect(0, 0, width, height);
	this.buffer_context.drawImage(this.getVideo(), 0, 0, width, height);
	// Apply blur
	StackBlur.canvasRGB(this.buffer.get(0), 0, 0, width, height, 15);

	this.initialSelection = true;
}

VideoController.prototype.enableCalipers = function() {
	this.resizeCanvas(this.caliperCanvas);
	this.calipers.enable();
	this.caliperCanvas.show();
}

VideoController.prototype.disableCalipers = function() {
	this.calipers.disable();
	this.resetCanvas(this.caliperCanvas);
	this.caliperDraw();

	if (this.length) {
		this.length.remove();
		this.length = null;
	}
}

VideoController.prototype.resizeCanvas = function(canvas) {
	var width = this.getWidth();
	var height = this.getHeight();

	// If region select is enabled, take width and height from 
	// the region canvas instead (as the video is shrunk)
	if (this.select.enabled != RegionSelect.DISABLED) {
		width = this.regionCanvas.attr("width");
		height = this.regionCanvas.attr("height");
	}

	var video = this.getVideo();
	// Fix container and canvas dimensions
	this.container.css({width: width, height: height, display: "block"});
	canvas.attr("width", width).attr("height", height);
}

VideoController.prototype.resetCanvas = function(canvas) {
	// Only reset if no other overlay canvas is being used
	if (this.select.enabled == RegionSelect.DISABLED && !this.calipers.enabled) {
		this.container.css({width: "auto", height: "auto", display: "table"});

	}
	this.video.css({position: "static"});
	canvas.hide();
}

VideoController.prototype.disableCrop = function() {
	this.drawBuffer(this.select.getSelection().left(),  this.select.getSelection().top());

	this.select.disable();

	var resetFunction = function() {
		this.resetCanvas(this.regionCanvas);
		this.regionDraw();
	}.bind(this);

	if (this.initialSelection) {
		resetFunction();
	} else {
		new VideoSizeListener(this.video, resetFunction);
	}
	
	this.cropCB(Selection.reset());
	this.initialSelection = false;
}

VideoController.prototype.regionDraw = function() {
	// Draw blurred video
	this.regionContext.drawImage(this.buffer.get(0), 0, 0, this.buffer.attr("width"), this.buffer.attr("height"));

	// Draw selection border + clear selected region
	this.select.draw(this.regionContext);
}

VideoController.prototype.caliperDraw = function() {
	// Draw selection border + clear selected region
	this.calipers.draw(this.caliperContext);
}

VideoController.prototype.drawBuffer = function(left, top) {
	// Draw the current video frame in the selection while waiting for stream to switch
	this.regionContext.save()
	this.regionContext.globalCompositeOperation = 'destination-over';
	this.regionContext.drawImage(this.video.get(0), left, top, this.getWidth(), this.getHeight());
	this.regionContext.restore();
}

VideoController.prototype.onNewSelection = function(selection) {
	if (selection.isReset()) {
		this.drawBuffer(this.select.getSelection().left(), this.select.getSelection().top());

		if (!this.initialSelection) {
			new VideoSizeListener(this.video, function() {
				this.video.css({position: "static"});
				this.regionDraw();
			}.bind(this));
		}
		this.cropCB(selection);
	} else {
		// Instead of showing the video in the selected region, 
		// show the current contents of the video, until the stream switches
		this.regionDraw();
		this.drawBuffer(0, 0);

		new VideoSizeListener(this.video, function(selection) {
			this.video.css({
				position: "absolute", 
				top: selection.top(), 
				left: selection.left(), 
			});
			this.regionDraw();
		}.bind(this, selection));
		this.cropCB(selection);
	}

	this.initialSelection = false;
}

VideoController.prototype.onLineChanged = function(line) {
	this.caliperDraw();

	if (line) this.updateLength(line.getLength());
}

VideoController.prototype.setPixelSize = function(size) {
	// The size in mm of each pixel
	this.pixelSize = size;
}

VideoController.prototype.setCalipersToggle = function(toggle) {
	this.toggleCalipers = toggle;
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