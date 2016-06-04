function Selection(x1, y1) {
    // First points which define the rectangular selection
    this._x1 = x1;
    this._y1 = y1;
    this._x2 = x1;
    this._y2 = y1;
}

Selection.reset = function() {
    return new Selection(0, 0);
}

Selection.prototype.update = function(x2, y2) {
    // Update the second points which define the rectangular selection
    this._x2 = x2;
    this._y2 = y2;
};


Selection.prototype.left = function() {
    return this._x1 < this._x2 ? this._x1 : this._x2;
};

Selection.prototype.right = function() {
    return this._x2 > this._x1 ? this._x2 : this._x1;
};

Selection.prototype.top = function() {
    return this._y1 < this._y2 ? this._y1 : this._y2;
};

Selection.prototype.bottom = function() {
    return this._y1 > this._y2 ? this._y1 : this._y2;
};

Selection.prototype.isReset = function() {
    return this._x1 == 0 && this._x2 == 0 &&
           this._y1 == 0 && this._y2 == 0;
};

function RegionSelect(canvas) {
    this.canvas = canvas;

    this.selection = null;
    this.mousedown = false;
    this.enabled = false;

    this.canvas.mousemove(this._on_mousemove.bind(this))
               .mousedown(this._on_mousedown.bind(this))
               .mouseup(this._on_mouseup.bind(this));

    this._request_draw();
}

RegionSelect.prototype.enable = function() {
    this.canvas.css("cursor", "crosshair");
    this.selection = Selection.reset();
    this.enabled = true;
};

RegionSelect.prototype.disable = function() {
    this.canvas.css("cursor", "default");
    this.enabled = false;
};

RegionSelect.prototype.disableAndClear = function() {
    this.disable();
    this.selection = null;
};


RegionSelect.prototype.draw = function(context) {
    if (!this.selection) return;

    // Draw borders by drawing shrinking rectangles
    var BORDER_COLOURS = ["#999999", "#ebebeb", "#999999"];
    var i;
    for (i = 0; i < 3; i++) {
        context.fillStyle = BORDER_COLOURS[i];
        context.fillRect(this.selection.left() + i, this.selection.top() + i,
                     this.selection.right() - this.selection.left() - i * 2, this.selection.bottom() - this.selection.top() - i * 2);
    }
    // And then clearing the middle
    context.clearRect(this.selection.left() + i, this.selection.top() + i,
                 this.selection.right() - this.selection.left() - i * 2, this.selection.bottom() - this.selection.top() - i * 2);
},

RegionSelect.prototype._on_mousemove = function(event) {
    if (!this.enabled) return;
    if (!this.mousedown) return;
    this._fix_event(event);

    this.selection.update(event.offsetX,  event.offsetY);
    this._request_draw();
},

RegionSelect.prototype._on_mousedown = function(event) {
    if (!this.enabled) return;
    this._fix_event(event);
    this.mousedown = true;

    // Reset
    this.newSelectionCB(Selection.reset());
    this.selection = new Selection(event.offsetX, event.offsetY);
},

RegionSelect.prototype._on_mouseup = function(event) {
    if (!this.enabled) return;
    this._fix_event(event);

    if (this.mousedown) {
        // Finish creating new selection
        this.selection.update(event.offsetX, event.offsetY);
        if (this.newSelectionCB) this.newSelectionCB(this.selection);
    }

    this.mousedown = false;
    this.disable();
}

RegionSelect.prototype._request_draw = function() {
    if (this.requestDrawCB) this.requestDrawCB();
}

RegionSelect.prototype.getSelection = function() {
    return this.selection;
}

RegionSelect.prototype._fix_event = function(event) {
    // For firefox from http://stackoverflow.com/questions/22716333/firefox-javascript-mousemove-not-the-same-as-jquery-mousemove
    if(typeof event.offsetX === "undefined" || 
       typeof event.offsetY === "undefined") {
       var targetOffset = $(event.target).offset();
       event.offsetX = event.pageX - targetOffset.left;
       event.offsetY = event.pageY - targetOffset.top;
    }
}

RegionSelect.prototype.requestDraw = function(cb) { this.requestDrawCB = cb; }
RegionSelect.prototype.newSelection = function(cb) { this.newSelectionCB = cb; }

