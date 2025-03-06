/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { message } from 'antd';

export class PollRequest {
    private timer: NodeJS.Timeout | null = null;

    constructor(step: number, request: (params?: object) => Promise<any>, callback: (data: any) => void) {
        this.start(step, request, callback);
    }

    async start(step: number, request: (params?: object) => Promise<any>, callback: (data: any) => void): Promise<void> {
        const f = async () => {
            try {
                const { result, msg, body } = await request();
                if (!result) {
                    message.error(msg);
                    return;
                } else {
                    callback(body);
                }
            } catch {
                // 避免因异常请求导致轮询停止
            };
        };
        await f();
        await this.poll(step, f);
    }

    async poll(step: number, f: () => Promise<void>): Promise<void> {
        this.timer = setTimeout(async () => {
            await f();
            this.timer = null;
            await this.poll(step, f);
        }, step);
    }

    clear(): void {
        // 销毁当前实例的方法，避免内部定时器无法取消
        this.poll = async () => { };
        this.start = async () => { };
        if (this.timer !== null) {
            clearTimeout(this.timer);
        }
    }
};

export const creatPollRequest = (step: number, request: (params?: object) => Promise<any>, callback: (data: any) => void): PollRequest => {
    return new PollRequest(step, request, callback);
};
