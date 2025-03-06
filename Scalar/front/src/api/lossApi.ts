/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { request } from './request';

interface ResponseBase<T> {
    result: boolean;
    body: T;
    msg?: string;
    errCode?: number;
};

interface ImportFileRequestParams {
    append: boolean;
    pathList: string[];
};
interface ImportFileResponseBody {
    data: FileItem[];
};
export interface FileItem {
    tag: string;
    fileList: Array<{ name: string; path: string }>;
};
export const importFile = async (params: ImportFileRequestParams): Promise<ResponseBase<ImportFileResponseBody>> => {
    return await request({
        data: params,
        url: '/ScalarVisually/ImportFile',
        method: 'post',
    });
};

export interface ChartsDataRequestParams {
    graphList: Array<{
        tag: string;
        file: string;
        offset: number;
        sampleOffset: number;
        sampleAlgorithm: string;
        sampleWeight: number;
    }>;
};
export interface ChartsDataResponseBody {
    data: ChartsDataItem[];
};
interface ChartsDataItem {
    tag: string;
    file: string;
    points: DataItem[];
    sampledPoints?: DataItem[];
};
export interface DataItem {
    step: number;
    value: number;
};

let controller = new AbortController();
export const getChartsData = async (params: ChartsDataRequestParams): Promise<ResponseBase<ChartsDataResponseBody>> => {
    controller.abort();
    controller = new AbortController();
    return await request({
        data: params,
        url: '/ScalarVisually/GetScalarData',
        method: 'post',
        signal: controller.signal,
    });
};

export interface NewFileResponseBody {
    data: NewFileItem[];
};
interface NewFileItem {
    dir: string;
    fileList: string[];
};
export const getNewFile = async (): Promise<ResponseBase<NewFileResponseBody>> => {
    return await request({
        url: '/ScalarVisually/GetAddFiles',
        method: 'post',
    });
};

export const getAllGraph = async (): Promise<ResponseBase<ImportFileResponseBody>> => {
    return await request({
        url: '/ScalarVisually/GetAllGraph',
        method: 'post',
    });
};
