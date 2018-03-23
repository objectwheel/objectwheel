function fitx(val) {
    return (val * dpi)
}

function fitf(val) {
    return Math.floor(val * dpi)
}

function fitc(val) {
    return Math.ceil(val * dpi)
}

function fit(val) {
    return fitf(val) < 1.0 ? fitc(val) : fitf(val)
}


