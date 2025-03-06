/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { observer } from 'mobx-react';
import React, { useEffect, useState } from 'react';
import {
    Checkbox,
    Form,
    message,
    Modal,
} from 'antd';
import { creatPollRequest, type PollRequest } from '@/utils/pollRequest';
import { type LossShowInfo } from '@/entity/lossShow';
import {
    importFile,
    getNewFile,
    type NewFileResponseBody,
} from '@/api/lossApi';
import { useTranslation } from 'react-i18next';
import eventBus from '@/eventBus';

let getChartsDataPoll: PollRequest;
const clearPollRequest = () => {
    if (getChartsDataPoll) {
        getChartsDataPoll.clear();
    }
};
const pollRequest = (f: ((data: any) => any) = () => { }) => {
    clearPollRequest();
    getChartsDataPoll = creatPollRequest(10000, getNewFile, f);
};

eventBus.on('removeFile', clearPollRequest);

export const UpLoadFile = observer(({ lossShowInfo }: { lossShowInfo: LossShowInfo; }): JSX.Element => {
    const handleData = (body: NewFileResponseBody) => {
        if (body.data.length > 0) {
            lossShowInfo.modifyNewFileList(body.data);
            clearPollRequest();
        }
    };

    const request = async (path: string) => {
        eventBus.emit('setLoading', true);
        const { result, msg, body } = await importFile({
            pathList: [path],
            append: false,
        });
        if (!result) {
            message.error(msg);
            return;
        }
        pollRequest(handleData);
        lossShowInfo.clearFileList();
        lossShowInfo.addFile(body.data);
        eventBus.emit('setLoading', false);
        eventBus.emit('updataChartData');
    };

    useEffect(() => {
        eventBus.on('importFile', request);
        return () => {
            eventBus.off('importFile', request);
        };
    });

    return <></>;
});

export const PollGetNewFile = observer(({ lossShowInfo }: { lossShowInfo: LossShowInfo }): JSX.Element => {
    const { t } = useTranslation('lossShow');
    const [showModal, setShowModal] = useState(false);
    const [form] = Form.useForm();

    const handleData = (body: NewFileResponseBody) => {
        if (body.data.length > 0) {
            lossShowInfo.modifyNewFileList(body.data);
            clearPollRequest();
        }
    };

    const handleOk = async () => {
        const pathList: string[] = [];
        const chackFile = form.getFieldsValue();
        Object.keys(chackFile).forEach(key => {
            chackFile[key].forEach((path: string) => {
                pathList.push(`${key}/${path}`);
            });
        });
        if (pathList.length === 0) {
            setShowModal(false);
            pollRequest(handleData);
            return;
        }
        const { result, msg, body } = await importFile({
            pathList,
            append: true,
        });
        if (!result) {
            message.error(msg);
            return;
        }
        lossShowInfo.addFile(body.data);
        setShowModal(false);
        pollRequest(handleData);
    };

    const handleCancel = () => {
        setShowModal(false);
        pollRequest(handleData);
    };

    useEffect(() => {
        if (lossShowInfo.newFileList.length > 0) {
            setShowModal(true);
            clearPollRequest();
        }
    }, [lossShowInfo.newFileList]);
    
    return <Modal title={t('folderChange')} open={showModal} onOk={handleOk} width="540px"
        onCancel={handleCancel} maskClosable={false}>
        <Form form={form} className="pollGetNewFileForm">
            {
                lossShowInfo.newFileList.map(item => (
                    <div key={item.dir}>
                        <div className="dir">{item.dir}</div>
                        <Form.Item name={item.dir} style={{ margin: 0, marginLeft: 10 }}>
                            <Checkbox.Group options={item.fileList} className="checkboxGroup" />
                        </Form.Item>
                    </div>
                ))
            }
        </Form>
    </Modal>;
});
