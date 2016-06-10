
function Line(x1, y1) {
    this.x1 = x1;
    this.y1 = y1;
}

Line.prototype.update = function(x2, y2) {
    this.x2 = x2;
    this.y2 = y2;
}

Line.prototype.getLength = function() {
    return Math.sqrt(Math.pow(this.x1 - this.x2,  2) + Math.pow(this.y1 - this.y2, 2));
}


function Calipers(canvas) {
    this.canvas = canvas;

    this.line = null;
    this.mousedown = false;
    this.enabled = false;

    this.canvas.mousemove(this._on_mousemove.bind(this))
               .mousedown(this._on_mousedown.bind(this))
               .mouseup(this._on_mouseup.bind(this));
}

Calipers.prototype.enable = function() {
    this.canvas.css("cursor", "crosshair");
    this.line = null;
    this.enabled = true;
};

Calipers.prototype.disable = function() {
    this.canvas.css("cursor", "default");
    this.enabled = false;
    this.line = null;
};


Calipers.prototype.draw = function(context) {
    if (!this.line) return;

    context.clearRect(0, 0, this.canvas.get(0).width, this.canvas.get(0).height);

    context.strokeStyle = "#1a1a1a";
    context.beginPath();
    context.moveTo(this.line.x1, this.line.y1);
    context.lineTo(this.line.x2, this.line.y2);
    context.stroke();

    context.strokeStyle = "#ebebeb";
    context.beginPath();
    context.moveTo(this.line.x1, this.line.y1-1);
    context.lineTo(this.line.x2, this.line.y2-1);
    context.stroke();
},

Calipers.prototype._on_mousemove = function(event) {
    if (!this.enabled) return;
    if (!this.mousedown) return;
    this._fix_event(event);

    this.line.update(event.offsetX,  event.offsetY);
    this.lineChangedCB(this.line);
},

Calipers.prototype._on_mousedown = function(event) {
    if (!this.enabled) return;
    this._fix_event(event);
    this.mousedown = true;

    // Reset
    this.lineChangedCB(null);
    this.line = new Line(event.offsetX, event.offsetY);
},

Calipers.prototype._on_mouseup = function(event) {
    if (!this.enabled) return;
    this._fix_event(event);

    if (this.mousedown) {
        // Finish creating new line
        this.line.update(event.offsetX, event.offsetY);
        this.lineChangedCB(this.line);
    }

    this.mousedown = false;
}

Calipers.prototype.getLine = function() {
    return this.line;
}

Calipers.prototype._fix_event = function(event) {
    // For firefox from http://stackoverflow.com/questions/22716333/firefox-javascript-mousemove-not-the-same-as-jquery-mousemove
    if(typeof event.offsetX === "undefined" || 
       typeof event.offsetY === "undefined") {
       var targetOffset = $(event.target).offset();
       event.offsetX = event.pageX - targetOffset.left;
       event.offsetY = event.pageY - targetOffset.top;
    }
}

Calipers.prototype.lineChanged = function(cb) { this.lineChangedCB = cb; }

