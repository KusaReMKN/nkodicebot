'use strict';

console.log('Dice for ', process.argv[2]);

let desc = '';

// dices の添字意味
const DICES = [ '＊', 'う', 'ま', 'ち', 'ん', 'こ', 'お', ];
// scores の添字意味
const SCORE = [ 'U', 'M', 'C', ];
// yaku の添字意味
const YAKU = {
	REGEX: /(おちんちん|おまんこ|ちんちん|ちんこ|まんこ|うんこ|うんち)/g,
	// 役の一覧
	YAKU: [
		'\x02\x03' + '4お\x03' + '7ち\x03' + '3ん\x03' + '12ち\x03' + '6ん\x03\x02',
		'\x02\x03' + '7ちんちん\x03\x02',
		'\x02\x03' + '13おまんこ\x03\x02',
		'\x03' + '13まんこ\x03',
		'\x03' + '7ちんこ\x03',
		'\x03' + '5うんこ\x03',
		'\x03' + '5うんち\x03',
	],
	// 役を構成する出目 (dices 準拠)
	DICES: [
		[ 0, 0, 0, 2, 2, 0, 1, ],	// おちんちん
		[ 0, 0, 0, 2, 2, 0, 0, ],	// ちんちん
		[ 0, 0, 1, 0, 1, 1, 1, ],	// おまんこ
		[ 0, 0, 1, 0, 1, 1, 0, ],	// まんこ
		[ 0, 0, 0, 1, 1, 1, 0, ],	// ちんこ
		[ 0, 1, 0, 0, 1, 1, 0, ],	// うんこ
		[ 0, 1, 0, 1, 1, 0, 0, ],	// うんち
	],
	// 役の並び順
	SORT: [
		[ 6, 3, 4, 3, 4, ],	// おちんちん
		[ 3, 4, 3, 4, ],	// ちんちん
		[ 6, 2, 4, 5, ],	// おまんこ
		[ 2, 4, 5, ],		// まんこ
		[ 3, 4, 5, ],		// ちんこ
		[ 1, 4, 5, ],		// うんこ
		[ 1, 4, 3, ],
	],
	// 役のスコア (SCORE 準拠)
	SCORE: [
		[ 10000,    0,    0, ],	// おちんちん
		[     0,    0, 3000, ],	// ちんちん
		[     0, 5000,    0, ],	// おまんこ
		[     0, 1000,    0, ],	// まんこ
		[  1000,    0,    0, ],	// ちんこ
		[     0,    0, 1000, ],	// うんこ
		[     0,    0, 1000, ],	// うんち
	],
};
// dices 準拠
const TRIPLES = [
	null,
	[  2.0,    0,    0, ],
	[    0,  2.0,    0, ],
	[    0,    0,  2.0, ],
	[ -3.0, -3.0, -3.0, ],
	[  1.5,  1.5,  1.5, ],
	[  1.5,  1.5,  1.5, ],
];

// 正規分布乱数生成器
const nrand = (() => {
	let flag = false;
	let r1, r2, s;
	return () => {
		flag = !flag;
		if (flag) {
			do {
				r1 = 2. * Math.random() - 1.;
				r2 = 2. * Math.random() - 1.;
				s = r1 * r1 + r2 * r2;
			} while (s > 1. || s === 0.);
			s = Math.sqrt(-2. * Math.log(s) / s);
			return r1 * s;
		}
		return r2 * s;
	};
})();

// サイコロを振る。
// out dices: サイコロの出目が格納される
// in  n:     サイコロの数
// return:    小便にならないサイコロの数
function Dice(dices, n) {
	let num;
	// 小便にならないサイコロの数を決める
	do { num = Math.floor(nrand() + n + 1); } while (num < 0 || n < num);
	// サイコロを振る
	for (let i = 0; i < n; i++)
		dices[i] = i < num ? Math.floor(Math.random() * 6 + 1) : 0;
	return num;
}

// 出目による点数の計算
// in  dices: サイコロの出目
// i/o score: スコアが加算される
// return:    なし
function DemeScore(dices, score) {
	const DemeScoreTable = [
		[ -500, -500, -500, ],	// 小便
		[  500,    0,    0, ],	// う
		[    0,  500,    0, ],	// ま
		[    0,    0,  500, ],	// ち
		[   50,   50,   50, ],	// ん
		[  100,  100,  100, ],	// こ
		[  300,  300,  300, ],	// お
	];
	for (let i = 0; i < dices.length; i++)
		for (let j = 0; j < SCORE.length; j++)
			score[j] += DemeScoreTable[dices[i]][j];
}

// 役による点数の計算
// in  dices: サイコロの出目
// i/o score: スコアが加算される
// i/o lyaku: 前回の役 (コンボ判定用)
// return: -1: 次のサイコロの数は 10
//          0: 数補正なし
//     正整数: ダイス追加
function YakuScore(dices, score, lyaku, roll) {
	// 役番号 y の役を満たしているか？
	function IsYaku(deme, y) {
		for (let i = 0; i < DICES.length; i++)
			if (deme[i] < YAKU.DICES[y][i]) return false
		return true;
	}
	// 役番号 y の役を清算する
	function CalcYaku(deme, y) {
		for (let i = 0; i < DICES.length; i++)
			deme[i] -= YAKU.DICES[y][i];
	}
	// コンボ倍率の計算
	function Combo(lyaku, y) {
		const k = lyaku[y];
		if (k === 1) return 1;
		if (k === 2) return 2;
		if (k === 3) return 4;
		return 8;
	}

	let YakuCount = 0;
	let theYakuCount = 0;
	let OchinchinFlag = false;
	let ChinchinDisabled = false, MankoDisabled = false;
	let deme = [ 0, 0, 0, 0, 0, 0, 0, ];
	for (let i = 0; i < dices.length; i++) deme[dices[i]]++;
	for (let i = 0; i < YAKU.YAKU.length; ) {
		if (i === 2 && ChinchinDisabled) i++;
		if (i === 4 && MankoDisabled) i++;
		if (IsYaku(deme, i)) {
			if (i === 0)
				ChinchinDisabled = true;
			if (i === 1)
				MankoDisabled = true;
			if (theYakuCount === 0) lyaku[i]++;
			YakuCount++; theYakuCount++;
			if (i === 0) OchinchinFlag = true;
			CalcYaku(deme, i);
			for (let j = 0; j < SCORE.length; j++)
				score[j] += YAKU.SCORE[i][j] * Combo(lyaku, i);
		} else {
			if (i !== 0 && i !== 2) {
				deme = [ 0, 0, 0, 0, 0, 0, 0, ];
				for (let i = 0; i < dices.length; i++) deme[dices[i]]++;
			}
			if (theYakuCount > 0) {
				desc += `${YAKU.YAKU[i]}`;
				if (lyaku[i] > 1)
					for (let j = 1; j < lyaku[i] && j < 4; j++)
						desc += '*';
				if (theYakuCount > 1)
					desc += `×${theYakuCount}`;
				desc += ` `;
			} else {
				lyaku[i] = 0;
			}
			theYakuCount = 0;
			i++;
		}
	}
	
	if (YakuCount === 0) desc += '役無し ';
	desc += '| ';
 	if (YakuCount > 0) roll[0]++;
	if (OchinchinFlag) return -1;
	if (YakuCount >= 2) return YakuCount - 1;
	return 0;
}

// ゾロ目による点数の計算
// in  dices: サイコロの出目
// i/o score: スコアが倍される
function ZoromeScore(dices, score) {
	let deme = [ 0, 0, 0, 0, 0, 0, 0, ];
	for (let i = 0; i < dices.length; i++) deme[dices[i]]++;
	for (let i = 1; i < DICES.length; i++) {
		if (deme[i] < 3) continue;
		let add = deme[i] - 3;
		if (add > 4) add = 4;
		
		desc += `${DICES[i]}×${deme[i]} `;
		for (let j = 0; j < SCORE.length; j++) {
			let k = TRIPLES[i][j];
			if (k === 0) continue;
			if (k < 0) k -= add;
			else k += add;
			score[j] *= k;
			if (i === 6 && score[j] < 0) score[j] = -score[j];
		}
	}
}

function CalcScore(dices, n, score, lyaku, roll) {
	DemeScore(dices, score);
	let k = YakuScore(dices, score, lyaku, roll);
	switch (k) {
		case 0: break;
		case -1: n = 10; break;
		default: n += k;
	}
	ZoromeScore(dices, score);
	return n;
}

async function Game() {
	let d = [], s = [ 0, 0, 0, ], y = [ 0, 0, 0, 0, 0, 0, 0, ];
	let n = 5, r = [ 3 ];
	while (n > 0 && r[0] > 0) {
		desc = '';
		d = [];
		Dice(d, n);
		n = CalcScore(d, n, s, y, r);
		if (n > 10) n = 10;

		let tmp = [];
		for (let i = 0; i < d.length; i++) {
			tmp.push(DICES[d[i]]);
		}
		r[0]--;
		if (!process.argv[3])
			await new Promise(r => setTimeout(() => r(), 1500));
		console.log(tmp.join(''), '|', desc);
	}
	console.log(`RESULT (${process.argv[2]}) \x02 `,
		Math.round(s[0] + s[1] + s[2]), ' \x02 ');
}
Game();
