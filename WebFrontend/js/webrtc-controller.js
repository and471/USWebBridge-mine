function WebRTCController() {
	this.RETRY_DELAY = 2000;
	this.paused = false;
}

WebRTCController.prototype.init = function() {
	Janus.init({
		debug: false, 
		callback: this.onJanusInit.bind(this)
	});
};

WebRTCController.prototype.onJanusInit = function() {
	if (!Janus.isWebrtcSupported()) {
		Janus.error("No WebRTC support... ");
		return;
	}

	var server = null;
	if (window.location.protocol === 'http:') {
		server = "http://" + window.location.hostname + ":8088/janus";
	} else {
		server = "https://" + window.location.hostname + ":8089/janus";
	}

	// Create session
	this.janus = new Janus({
		server: server,
		success: this.onJanusSuccess.bind(this),
		error: this.onJanusError.bind(this),
		destroyed: window.location.reload
	});
};

WebRTCController.prototype.onJanusError = function(error) {
	Janus.error(error);
	Janus.error("Retrying in " + this.RETRY_DELAY + " milliseconds");
	this.janus.destroy();

	// Retry on error
	setTimeout(this.onJanusInit.bind(this), this.RETRY_DELAY);
}

WebRTCController.prototype.onJanusSuccess = function() {
	this.janus.attach({
		plugin: "plugin.ultrasound",
		success: this.onPluginSuccess.bind(this),
		error: this.onJanusError.bind(this), 
		onmessage: this.onMessage.bind(this),
		ondataopen: this.onDataOpen.bind(this),
		ondata: this.onData.bind(this),
		onremotestream: this.remoteStreamCB
	});
};

WebRTCController.prototype.onDataOpen = function(data) {
	this.plugin.send({"message": {"request": "ready"}});
	$("#main").removeClass("disabled");
}


WebRTCController.prototype.onPluginSuccess = function(plugin) {
	this.plugin = plugin;
	Janus.log("Plugin attached! (" + plugin.getPlugin() + ", id=" + plugin.getId() + ")");

	this.pluginSuccessCB();
}

WebRTCController.prototype.onMessage = function(msg, jsep) {
	if (msg["error"] !== undefined && msg["error"] !== null) {

		if (msg["error"] == "AUTH_FAIL") {
			this.login_modal.fail();
			return;
		}

		stopStream();
		return;
	}

	var result = msg["result"];
	if (result !== null && result !== undefined && result["status"] !== undefined && result["status"] !== null) {
		this.handleStatus(result["status"]);
	} 
	if (jsep !== undefined && jsep !== null) {
		this.handleJSEP(jsep);
	}
}

WebRTCController.prototype.handleStatus = function(status) {
	if (status === 'preparing') {
		this.authenticationSuccessCB();
	}/* else if (status === 'starting') { 
		$('#video-status').removeClass('hide').text("Starting...").show();
	} else if (status === 'started') {
		$('#video-status').addClass('hide').show();
	} */else if (status === 'stopped') {
		this.stopStream();
	}
}


WebRTCController.prototype.handleJSEP = function(jsep) {
	// Answer
	this.plugin.createAnswer(
		{
			jsep: jsep,
			media: { audioSend: false, videoSend: false, data: true },	// We want recvonly audio/video
			success: function(jsep) {
				this.plugin.send({"message": {"request": "start"}, "jsep": jsep});
			}.bind(this),
			error: function(error) {
				Janus.error("WebRTC error:", error);
			}
		});
}

WebRTCController.prototype.stopStream = function() {
	this.plugin.send({"message": {"request": "stop"}});
	this.plugin.hangup();
	this.janus.destroy();
}

WebRTCController.prototype.togglePause = function() {
	this.paused = !this.paused;
};

WebRTCController.prototype.sendData = function(data) {
	this.plugin.data({
		text: JSON.stringify(data),
		error: function(reason) { bootbox.alert(reason); },
		success: function() { $('#datasend').val(''); },
	});
}

WebRTCController.prototype.watch = function(auth) {
	this.plugin.send({"message": {
		"request": "watch", 
		"auth": auth
	}});
}

WebRTCController.prototype.onData = function(data_str) {
	if (this.paused) return;

	var data = JSON.parse(data_str);
	switch (data.method) {
		case "NEW_PATIENT_METADATA":
			if (this.newPatientMetadataCB) this.newPatientMetadataCB(data["data"]);
			break;
		case "NEW_IMAGE_METADATA":
			if (this.newImageMetadataCB) this.newImageMetadataCB(data["data"]);
			break;
		case "N_SLICES_CHANGED":
			if (this.nSlicesChangedCB) this.nSlicesChangedCB(data["data"]["nSlices"]);
			break;
	}
}
WebRTCController.prototype.newImageMetadata = function(cb) { this.newImageMetadataCB = cb; }
WebRTCController.prototype.newPatientMetadata = function(cb) { this.newPatientMetadataCB = cb; }
WebRTCController.prototype.nSlicesChanged = function(cb) { this.nSlicesChangedCB = cb; }
WebRTCController.prototype.authenticationSuccess = function(cb) { this.authenticationSuccessCB = cb; }
WebRTCController.prototype.pluginSuccess = function(cb) { this.pluginSuccessCB = cb; }
WebRTCController.prototype.remoteStream = function(cb) { this.remoteStreamCB = cb; }

