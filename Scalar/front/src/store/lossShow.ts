/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { makeAutoObservable } from 'mobx';
import { LossShowInfo } from '@/entity/lossShow';

export class LossShowStore {
    private _activeLossShow: LossShowInfo | undefined;
    constructor() {
        makeAutoObservable(this);
        this._activeLossShow = new LossShowInfo();
    }
    get activeLossShow() {
        return this._activeLossShow;
    }
    set activeLossShow(value) {
        this._activeLossShow = value;
    }
    async newLossShow(){
        const lossShow = new LossShowInfo();
        return lossShow;
    }
};
