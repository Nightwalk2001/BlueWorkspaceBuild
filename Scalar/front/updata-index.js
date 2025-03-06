/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

// 此脚本为了删除vite打包后对esmodule模式兼容的部分
import fs from 'fs';

const distPath = './dist/index.html';
const htmlText = fs.readFileSync(distPath, 'utf-8');
let resultText = '';
const htmlArr = htmlText.match(/.*\n/g) || [];
htmlArr.forEach(str => {
    str = str.replace(/\s?nomodule\s?/g, ' ');
    str = str.replace(/\s?crossorigin\s?/g, ' ');
    str = str.replace(/data-src/g, 'src');
    str = str.replace(/System.import\([\s\S]*getAttribute\([\s\S]*\)\)/g, '');
    if (!/type="module"/i.test(str)) {
        resultText += str;
    }
});
fs.writeFileSync(distPath, resultText, 'utf-8');
console.log('build end');
