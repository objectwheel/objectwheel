var fileManager
var initialized = false
var fmString = "import com.objectwheel.components 1.0 \n FileManager {}"

function init() {
    if (!initialized) {
        initialized = true
        fileManager = Qt.createQmlObject(fmString, root)
    }
}

function mkfile(name) {
    init()
    return fileManager.mkfile(name)
}

function mkdir(name) {
    init()
    return fileManager.mkdir(name)
}

function rm(name) {
    init()
    return fileManager.rm(name)
}

function rn(from, to) {
    init()
    return fileManager.mv(from, to)
}

function rmsuffix(dir, suffix) {
    init()
    return fileManager.rmsuffix(dir, suffix)
}

function exists(name) {
    init()
    return fileManager.exists(name)
}

function mv(from, to) {
    init()
    return fileManager.mv(from, to)
}

function cp(from, toDir) {
    init()
    return fileManager.cp(from, toDir)
}

function isfile(name) {
    init()
    return fileManager.isfile(name)
}

function isdir(name) {
    init()
    return fileManager.isdir(name)
}

function rdfile(file) {
    init()
    return fileManager.rdfile(file)
}

function wrfile(file, data) {
    init()
    return fileManager.wrfile(file, data)
}

function dlfile(url) {
    init()
    return fileManager.dlfile(url)
}

function fname(name) {
    init()
    return fileManager.fname(name)
}

function dname(name) {
    init()
    return fileManager.dname(name)
}

function suffix(name) {
    init()
    return fileManager.suffix(name)
}

function separator() {
    init()
    return fileManager.separator()
}

function ls(dir) {
    init()
    return fileManager.ls(dir)
}

function lsdir(dir) {
    init()
    return fileManager.lsdir(dir)
}

function lsfile(dir) {
    init()
    return fileManager.lsfile(dir)
}

function ftype(file) {
    return stype(suffix(file))
}

function stype(suffix) {
    suffix = suffix.toLowerCase()
    if        ((suffix === "png") ||
               (suffix === "jpg") ||
               (suffix === "jpeg") ||
               (suffix === "tiff") ||
               (suffix === "gif") ||
               (suffix === "svg") ||
               (suffix === "bmp") ||
               (suffix === "tiff")) {
        return "img"
    } else if ((suffix === "txt") ||
               (suffix === "text") ||
               (suffix === "htm") ||
               (suffix === "html") ||
               (suffix === "css") ||
               (suffix === "java") ||
               (suffix === "cpp") ||
               (suffix === "c") ||
               (suffix === "cc") ||
               (suffix === "c++") ||
               (suffix === "cxx") ||
               (suffix === "m") ||
               (suffix === "mm") ||
               (suffix === "hpp") ||
               (suffix === "h") ||
               (suffix === "hh") ||
               (suffix === "py") ||
               (suffix === "json") ||
               (suffix === "xml") ||
               (suffix === "php") ||
               (suffix === "asp") ||
               (suffix === "lua") ||
               (suffix === "prl") ||
               (suffix === "sh") ||
               (suffix === "coffee") ||
               (suffix === "diff")) {
        return "txt"
    } else if ((suffix === "js")) {
        return "js"
    } else if ((suffix === "qml")) {
        return "qml"
    } else {
        return "bin"
    }
}

