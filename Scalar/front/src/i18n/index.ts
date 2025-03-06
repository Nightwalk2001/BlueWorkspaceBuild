/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import i18n from 'i18next';
import { initReactI18next } from 'react-i18next';
import { lossShowEn, lossShowZh } from './lossShow';
import en from './en.json';
import zh from './zh.json';

const resources = {
    enUS: {
        ...en,
        ...lossShowEn,
    },
    zhCN: {
        ...zh,
        ...lossShowZh,
    }
};

i18n.use(initReactI18next).init({
    resources,
    lng: 'enUS',
    interpolation: {
        escapeValue: false,
    },
});

export default i18n;
