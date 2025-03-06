/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import React, { useEffect, useState } from 'react';
import { observer } from 'mobx-react';
import { PollGetNewFile, UpLoadFile } from './Upload';
import { ChartsContainer, ChartsContainerTagAggregation } from './Chart';
import './index.scss';
import { getChartsData, type ChartsDataResponseBody } from '@/api/lossApi';
import type { FileListResult, LossShowInfo } from '@/entity/lossShow';
import { Checkbox, InputNumber, Select, type SelectProps } from 'antd';
import { creatPollRequest, type PollRequest } from '@/utils/pollRequest';
import { useTranslation } from 'react-i18next';
import eventBus from '@/eventBus';
import type { CheckboxChangeEvent } from 'antd/es/checkbox';

let getChartsDataPoll: PollRequest;

const clearPollRequest = () => {
    if (getChartsDataPoll) {
        getChartsDataPoll.clear();
    }
};
eventBus.on('removeFile', clearPollRequest);

export const LossContainer = observer(({ lossShowInfo }: { lossShowInfo: LossShowInfo }) => {
    const [listenFile, setListenFile] = useState(false);

    const getData = async () => {
        return await getChartsData({ graphList: lossShowInfo.getGraphList() });
    };

    const handleData = (body: ChartsDataResponseBody) => {
        lossShowInfo.addData(body.data);
    };

    const pollRequest = () => {
        getChartsDataPoll?.clear();
        getChartsDataPoll = creatPollRequest(lossShowInfo.stepGetData * 1000, getData, handleData);
        setListenFile(true);
    };

    useEffect(() => {
        eventBus.on('updataChartData', pollRequest);
        return () => {
            eventBus.off('updataChartData', pollRequest);
            getChartsDataPoll?.clear();
        };
    }, []);

    return (
        <>
            <div className="lossContainer">
                <HeaderOptions lossShowInfo={lossShowInfo} />
                <div className="main">
                    {
                        lossShowInfo.isTagPolymerize
                            ? <ChartsContainerTagAggregation lossShowInfo={lossShowInfo} />
                            : <ChartsContainer lossShowInfo={lossShowInfo} />
                    }
                </div>
            </div>
            {listenFile && <PollGetNewFile lossShowInfo={lossShowInfo} />}
        </>
    );
});

const HeaderOptions = observer(({ lossShowInfo }: { lossShowInfo: LossShowInfo }): JSX.Element => {
    const { t } = useTranslation('lossShow');
    const [options, setOptions] = useState([] as SelectProps['options']);
    
    const getSelectOptions = (fileList: FileListResult[]) => {
        return fileList.map(item => ({ label: item.tag, value: item.tag }));
    };

    const handleChange = (value: string[]) => {
        lossShowInfo.modifyShowTagList(value);
    };

    const onChange = (value: number | null) => {
        const num = value === null ? 1 : value;
        lossShowInfo.modifyStepGetData(num);

        if (lossShowInfo.getTagList().length > 0) {
            eventBus.emit('updataChartData');
        }
    };

    const init = () => {
        const fileList = lossShowInfo.getFileListByModel('isChecked');
        setOptions(getSelectOptions(fileList));
    };

    useEffect(() => {
        init();
    }, [lossShowInfo.fileListChange]);

    return <>
        <div className='tagSelect'>
            <div className="item">
                <div className="label">{t('tagList')}：</div>
                <div className="select">
                    <Select mode="tags" allowClear style={{ width: '100%' }} placeholder={t('tagListplaceholder')} value={lossShowInfo.showTagList}
                        onChange={handleChange} options={options} maxTagCount={'responsive'} />
                </div>
            </div>
            <div className="item">
                <div className="label">{t('tagAggregation')}：</div>
                <div className="checkBox">
                    <Checkbox onChange={(e: CheckboxChangeEvent) => lossShowInfo.modifyIsTagPolymerize(e.target.checked)}
                        checked={lossShowInfo.isTagPolymerize} />
                </div>
            </div>
            <div className="item">
                <div className="label">{t('updateFrequency')}：</div>
                <div className="numInput">
                    <InputNumber min={1} max={10} step={1} precision={0} onChange={onChange} value={lossShowInfo.stepGetData} style={{ width: '100%' }} />
                </div>
            </div>

            <div className="upload">
                <UpLoadFile lossShowInfo={lossShowInfo} />
            </div>
        </div>
    </>;
});
