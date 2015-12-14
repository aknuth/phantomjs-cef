(function() {

	phantom.Fs = function() {

	  this.write = function(file,s,m) {
		  native function write();
		  return write(file,s,m);
	  };
	  
	  this.read = function(file) {
		  native function read();
		  return read(file);
	  };
	  
	  this.lastModified = function(file) {
		  native function lastModified();
		  return lastModified(file);
	  };

	  this.exists = function(file) {
		  native function exists();
		  return exists(file);
	  };
	  this.isFile = function(file) {
		  native function isFile();
		  return isFile(file);
	  };
	  this.isDirectory = function(file) {
		  native function isDirectory();
		  return isDirectory(file);
	  };
	  
	  this.list = function(file) {
		  native function list();
		  return list(file);
	  };
	
	  this.copy = function(src,dest) {
		  native function copy();
		  return copy(src,dest);
	  };
	  
	  this.remove = function(path) {
		  native function remove();
		  return remove(path);
	  };
	  
	  this.makeDirectory = function(path) {
		  native function makeDirectory();
		  return makeDirectory(path);
	  };
	  
	  this.touch = function(file) {
		  native function touch();
		  return touch(file);
	  };
	  this.size = function(file) {
		  native function size();
		  return size(file);
	  };
	  this.tempPath = function(file) {
		  native function tempPath();
		  return tempPath(file);
	  };

	}

})();