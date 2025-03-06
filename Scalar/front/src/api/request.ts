/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import axios from 'axios';
import i18n from '@/i18n';

const isProduction = import.meta.env.VITE_MODE === 'production';

let baseURL: string = '';
export const setBaseURL = (url: string) => {
    const [protocol, URL] = url.split('://');
    baseURL = `${protocol === 'ws' ? 'http' : 'https'}://${URL}`;
};
const getBaseURL = () => {
    if (isProduction) {
        return baseURL;
    } else {
        return import.meta.env.VITE_URL;
    }
};

const requestBase = axios.create({
    baseURL: '',
    timeout: 10000,
});

requestBase.interceptors.request.use(
    config => {
        config.headers['content-type'] = 'text/plain';
        config.url = `${getBaseURL()}${config.url}`;
        return config;
    },
    error => {
        return Promise.reject(error);
    }
);

requestBase.interceptors.response.use(
    config => {
        return config;
    },
    error => {
        return Promise.reject(error);
    }
);

interface RequestOpts {
    url: string;
    method: string;
    data?: object;
    params?: object;
    signal?: AbortSignal;
};

export const request = async (requestOpts: RequestOpts) => {
    try {
        const res = await requestBase(requestOpts);
        return res.data;
    } catch (err: any) {
        /*
            请求异常处理
            如果是取消请求跳过
            如果是其它异常，抛出异常的请求
        */
        if (err.message !== 'canceled') {
            return { result: false, msg: `${i18n.t('common:requestFail')}：${requestOpts.url}` };
        }
    }
};
