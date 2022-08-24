var MP4Decoder = (function reader() {
    function constructor(rgb, canvas) {
        var decoder = new Decoder({
            rgb: rgb,
        });

        if (canvas) {
            this.canvas = canvas;
            const ctx = canvas.getContext('2d');
            decoder.ctx = ctx;
        }

        var onPictureDecoded = function (buffer, width, height, infos) {
            if (infos && infos.length > 0) {
                const info = infos[0];
                if (info.hasOwnProperty('resolve')) {
                    info.resolve({image: {data: buffer, width: width, height: height}, index: info.index});
                }
            }
        }

        decoder.onPictureDecoded = onPictureDecoded;

        this.decoder = decoder;
        this.decode = function (parData, parInfo) {
            decoder.decode(parData, parInfo);
        };
    }

    constructor.prototype = {
        load: function (bytes) {
            this._stream = new Bytestream(bytes);
            this._reader = new MP4Reader(this._stream);
            this._reader.read();

            var videoTrack = this._reader.tracks[1];
            this._length = videoTrack.getTotalTimeInSeconds();
            this._sampleSize = videoTrack.getSampleCount();

            var avc = videoTrack.trak.mdia.minf.stbl.stsd.avc1.avcC;
            var sps = avc.sps[0];
            var pps = avc.pps[0];

            this.decode(sps);
            this.decode(pps);
        },

        lastSeek: 0,

        videoTrack: function () {
            return this._reader.tracks[1];
        },

        decodeSeq: function (sample, resolve, reject) {
            if (this.lastSeek > sample) {
                this.lastSeek = 0;
            }

            var videoTrack = this.videoTrack();
            var decoder = this.decoder;
            while (this.lastSeek < sample) {
                decoder.infoAr.push({ finishDecoding: 0, index: this.lastSeek, resolve: resolve, reject: reject });
                videoTrack.getSampleNALUnits(this.lastSeek).forEach(function (nal) {
                    decoder.decode(nal);
                });
                this.lastSeek += 1;
            }
        },

        seek: function (offsetInMS) {
            var videoTrack = this.videoTrack();
            var offset = (offsetInMS / 1000.0) % this._length;
            var sample = videoTrack.timeToSample(videoTrack.secondsToTime(offset));

            var player = this;
            return new Promise((resolve, reject) => {
                player.decodeSeq(sample, resolve, reject);
            });
        }
    }

    return constructor;
})();

var MP4Player2 = (function reader() {
    var defaultConfig = {
        filter: "original",
        filterHorLuma: "optimized",
        filterVerLumaEdge: "optimized",
        getBoundaryStrengthsA: "optimized"
    };

    function constructor(stream, useWorkers, webgl, canvas) {
        this.stream = stream;
        this.useWorkers = useWorkers;
        this.webgl = webgl;

        var rgbDecoder = new MP4Decoder(false);

        if (canvas) {
            this.canvas = canvas;
            const ctx = canvas.getContext('2d');
            rgbDecoder.ctx = ctx;
        }

        this._rgbDecoder = rgbDecoder;
    }

    constructor.prototype = {
        load: function (rgbBytes, alphaBytes) {
            this._rgbDecoder.load(rgbBytes);
            if (alphaBytes) {
                this._alphaDecoder = new MP4Decoder(false);
                this._alphaDecoder.load(alphaBytes);
            }
            else {
                this._alphaDecoder = undefined;
            }
        },

        lastSeek: 0,

        seek: function (offsetInMS) {
            var promises = [this._rgbDecoder.seek(offsetInMS)];
            if (this._alphaDecoder) {
                promises.push(this._alphaDecoder.seek(offsetInMS));
            }

            return Promise.all(promises).then(imgs => {
                if (imgs.length > 1) {
                    const width = imgs[0].image.width;
                    const height = imgs[0].image.height;

                    const nv12ptr = DecoderModule._malloc(width * height * 3 / 2);
                    DecoderModule.HEAPU8.set(imgs[0].image.data, nv12ptr);

                    const alphaptr = DecoderModule._malloc(width * height);
                    DecoderModule.HEAPU8.set(imgs[1].image.data.subarray(0, width * height), alphaptr);

                    const rgbaptr = DecoderModule._malloc(width * height * 4);
                    DecoderModule._NV12Alpha2RGBA(width, height, nv12ptr, alphaptr, rgbaptr);
                    var newImg = new Uint8Array(DecoderModule.HEAPU8.buffer, rgbaptr, width * height * 4);
                    var imgData = new ImageData(new Uint8ClampedArray(newImg), width, height);

                    DecoderModule._free(nv12ptr);
                    DecoderModule._free(alphaptr);
                    DecoderModule._free(rgbaptr);
                    return createImageBitmap(imgData);
                }
            });
        }
    };

    return constructor;
})();
