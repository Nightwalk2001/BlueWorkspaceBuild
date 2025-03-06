/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { useCallback, useEffect, useRef } from 'react';

// 将输入转为不小于0的数
export const notZero = (num: number, replace = 1): number => {
    const replaceNum = replace === 0 ? 1 : replace;
    return num === 0 ? replaceNum : num;
};

// 按照第二位输入处理小数
export const keepDecimals = (num: number, n = 2): number => {
    let ratio: number;
    if (n < 0) {
        ratio = 0;
    } else if (n > 14) {
        ratio = 14;
    } else {
        ratio = n;
    }
    return Number(num.toFixed(ratio));
};

// 根据入参，将表格数据下载为csv文件
declare const window: Window & { showSaveFilePicker: any };
export interface HeaderType {
    key: string;
    title: string;
};
export const downloadCsv = async (header: HeaderType[], data: any[], fileName: string = '导出结果.csv'): Promise<void> => {
    if (header.length < 1 || data.length < 1) {
        return;
    }
    
    let csvContent = '';
    data.forEach((item, index) => {
        let dataString = '';
        for (let i = 0; i < header.length; i++) {
            dataString = `${dataString}${item[header[i].key]},`;
        }
        csvContent = `${csvContent}${index < data.length ? dataString.replace(/,$/, '\n') : dataString.replace(/,$/, '')}`;
    });
    const tableHeader = header.map(item => item.title).join(',');
    // 新api，可以选择文件的下载位置，仅部分浏览器版本支持
    if (typeof window?.showSaveFilePicker !== 'undefined') {
        csvContent = `${tableHeader}\n${csvContent}`;
        const newHandle = await window.showSaveFilePicker({ suggestedName: fileName });
        const writableStream = await newHandle.createWritable();
        await writableStream.write(new Blob([csvContent]));
        await writableStream.close();
    } else {
        csvContent = `data:text/csv;charset=utf-8,\ufeff${tableHeader}\n${csvContent}`;
        const a = document.createElement('a');
        a.href = encodeURI(csvContent);
        a.download = fileName;
        a.click();
        a.remove();
    }
};

const removePrototype = (obj: any) => {
    if (typeof obj !== 'object') {
        return;
    }
    for (const key in obj) {
        if (key === '__proto__' || key === 'constructor') {
            delete obj[key];
        } else if (typeof obj === 'object') {
            removePrototype(obj[key]);
        }
    }
};
export const safeJSONParse = (str: any, defaultValue: any = null) => {
    try {
        const res = JSON.parse(str);
        removePrototype(res);
        return res;
    } catch {
        return defaultValue;
    }
};

// 防抖函数
export interface UseRefParams {
    fn: (_args: any) => void;
    timer: ReturnType<typeof setTimeout> | null;
}
export const useDebounce = (fn: (_args?: any) => void, delay = 500) => {
    const { current } = useRef<UseRefParams>({ fn, timer: null });
    useEffect(() => {
        current.fn = fn;
    }, [current, fn]);
    return useCallback(
        (args?: any) => {
            if (current.timer) {
                clearTimeout(current.timer);
            }
            current.timer = setTimeout(() => {
                current.fn(args);
            }, delay);
        },
        [current, delay]
    );
};

// 获取数据图表折线颜色
const CHART_COLOR = ['rgba(84, 112, 198, 0.4)', 'rgba(84, 112, 198, 1)', 'rgba(145, 204, 117, 0.4)', 'rgba(145, 204, 117, 1)'];

export const getChartColor = (index: number) => {
    return CHART_COLOR[index];
};
