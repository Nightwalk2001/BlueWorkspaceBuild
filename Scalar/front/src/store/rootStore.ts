/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { LossShowStore } from './lossShow';

export class RootStore {
    lossShowStore: LossShowStore;
    constructor() {
        this.lossShowStore = new LossShowStore();
    }
    resetStore = () => {
        this.lossShowStore = new LossShowStore();
    };
};

export const store = new RootStore();
