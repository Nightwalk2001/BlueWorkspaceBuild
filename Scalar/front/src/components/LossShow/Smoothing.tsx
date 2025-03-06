/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import React, { useEffect, useState } from 'react';
import { InputNumber, Select, Slider } from 'antd';
import { useTranslation } from 'react-i18next';
import { useDebounce } from '@/utils/common';
import { LossShowInfo } from '@/entity/lossShow';
import eventBus from '@/eventBus';

export const Smoothing = ({ lossShowInfo, tag }: { lossShowInfo: LossShowInfo; tag: string | string[] }): JSX.Element => {
    const { t } = useTranslation('lossShow');
    const [algorithmValue, setAlgorithmValue] = useState('');
    const [rateValue, setRateValue] = useState(0);
    const algorithmOptions = [
        {
            label: 'First Order IR Sample',
            value: 'smoothing',
        }
    ];

    const algorithmChange = (value: string) => {
        setAlgorithmValue(value);
        debounceUpdataSmoothingConfig();
    };

    const rateChange = (value: number | null) => {
        if (Number.isNaN(value) || value === null) {
            return;
        }
        setRateValue(value);
        debounceUpdataSmoothingConfig();
    };

    const debounceUpdataSmoothingConfig = useDebounce(() => {
        if (algorithmValue === '') {
            return;
        }
        lossShowInfo.modifySmoothingConfig(tag, { sampleAlgorithm: algorithmValue, sampleWeight: rateValue });
        eventBus.emit('updataChartData');
    }, 500);

    const init = () => {
        const { sampleAlgorithm, sampleWeight } = lossShowInfo.getSmothingConfig(tag);
        setAlgorithmValue(sampleAlgorithm);
        setRateValue(sampleWeight);
        debounceUpdataSmoothingConfig();
    };

    useEffect(() => {
        init();
    }, []);

    return <div className="smoothingConfig">
        <div className="item">
            <div className="label">{t('algorithm')}：</div>
            <div className="select">
                <Select style={{ width: '100%' }} placeholder={t('algorithmPlaceholder')} value={algorithmValue}
                    onChange={algorithmChange} options={algorithmOptions} maxTagCount={'responsive'} size="small" />
            </div>
        </div>
        <div className="item">
            <Slider min={0} max={0.999} step={0.001} value={rateValue} onChange={rateChange}
                style={{ flex: 1, margin: '0 12px' }} />
            <InputNumber min={0} max={0.999} step={0.001} value={rateValue}
                onChange={rateChange} size="small" style={{ width: 70 }} />
        </div>
    </div>;
};