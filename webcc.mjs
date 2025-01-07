#!/usr/bin/env node
import fs from 'fs';
import path from 'path';
const __dirname = import.meta.dirname;
const wasm = fs.readFileSync(path.join(__dirname, "webcc.wasm"));
const mem = new WebAssembly.Memory({ initial: 16 });
const prog = await WebAssembly.instantiate(wasm, {
	env: {
		memory: mem,
	},
});
console.log(prog.instance.exports);
