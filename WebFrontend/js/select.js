function Selection(x1, y1) {
    // First points which define the rectangular selection
    this._x1 = x1;
    this._y1 = y1;
    this._x2 = x1;
    this._y2 = y1;
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

function RegionSelect(canvas) {
    this.canvas = canvas;
    this.context = this.canvas.get(0).getContext("2d");

    this.selection = null;
    this.mousedown = false;
    this.mouseover = false;
    this.enabled = false;

    this.canvas.mousemove(this._on_mousemove.bind(this))
               .mousedown(this._on_mousedown.bind(this))
               .mouseup(this._on_mouseup.bind(this))
               .mouseenter(this.onMouseEnter.bind(this))
               .mouseleave(this.onMouseLeave.bind(this));

    // Prevent focus border
    this.canvas.bind('selectstart', function(e) { e.preventDefault(); return false; })

    this._draw();
}

RegionSelect.prototype.enable = function() {
    this.selection = null;
    this.enabled = true;
};

RegionSelect.prototype.disable = function() {
    this.selection = null;
    this.enabled = false;
};


RegionSelect.prototype._draw = function(outline) {
    if (!this.enabled) return;
    if (!this.selection) return;

    if (this.drawCB) this.drawCB();

    this.context.save();
    this.context.clearRect(
        this.selection.left(), this.selection.top(), 
        this.selection.right() - this.selection.left(), 
        this.selection.bottom() - this.selection.top()
    );
    this.context.restore();


    //if (this.mouseover) {
        var BORDER_COLOURS = ["#999999", "#ebebeb", "#999999"];
        var i;
        for (i = 0; i < 3; i++) {
            this.context.fillStyle = BORDER_COLOURS[i];
            this.context.fillRect(this.selection.left() + i, this.selection.top() + i,
                         this.selection.right() - this.selection.left() - i * 2, this.selection.bottom() - this.selection.top() - i * 2);
        }

        this.context.clearRect(this.selection.left() + i, this.selection.top() + i,
                     this.selection.right() - this.selection.left() - i * 2, this.selection.bottom() - this.selection.top() - i * 2);
    //}


},

RegionSelect.prototype._on_mousemove = function(event) {
    if (!this.enabled) return;
    if (!this.mousedown) return;
    this._fix_event(event);

    this.selection.update(event.offsetX,  event.offsetY);

    console.log("(" + this.selection.left() +") - ("+ this.selection.right() +"),  (" + this.selection.top() +") - ("+ this.selection.bottom() +")")

    this._draw();
},

RegionSelect.prototype._on_mousedown = function(event) {
    if (!this.enabled) return;
    this._fix_event(event);
    this.mousedown = true;

    this.selection = new Selection(event.offsetX, event.offsetY);
},

RegionSelect.prototype._on_mouseup = function(event) {
    if (!this.enabled) return;
    this._fix_event(event);
    this.mousedown = false;

    if (this.newSelectionCB) this.newSelectionCB(this.selection);

    this._draw();
},

RegionSelect.prototype.getSelection = function() {
    return this.selection;
},

RegionSelect.prototype._fix_event = function(event) {
    // For firefox from http://stackoverflow.com/questions/22716333/firefox-javascript-mousemove-not-the-same-as-jquery-mousemove
    if(typeof event.offsetX === "undefined" || 
       typeof event.offsetY === "undefined") {
       var targetOffset = $(event.target).offset();
       event.offsetX = event.pageX - targetOffset.left;
       event.offsetY = event.pageY - targetOffset.top;
    }
}

RegionSelect.prototype.onMouseEnter = function() { 
    this.mouseover = true;
    this._draw();
}

RegionSelect.prototype.onMouseLeave = function() { 
    if (!this.mousedown) {
        this.mouseover = false;
        this._draw();
    }
}

RegionSelect.prototype.draw = function(cb) { this.drawCB = cb; }
RegionSelect.prototype.newSelection = function(cb) { this.newSelectionCB = cb; }

