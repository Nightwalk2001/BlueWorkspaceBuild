/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import type {
    ChartsDataResponseBody,
    ChartsDataRequestParams,
    DataItem,
    FileItem,
    NewFileResponseBody,
} from '@/api/lossApi';
import { makeAutoObservable } from 'mobx';

interface FileInfoItem {
    [key: string]: string | number | boolean | DataItem[];
    name: string;
    data: DataItem[];
    offset: number;
    isChecked: boolean;
    smoothingData: DataItem[];
    sampleOffset: number;
};
interface TagItem {
    [key: string]: FileInfoItem;
};
interface FileList {
    [key: string]: TagItem;
};
export interface FileListResult {
    tag: string;
    file: {
        filePath: string;
        fileName: string;
        value: string | number | boolean | DataItem[];
    }[];
};
export interface CheckListItem {
    tag: string;
    filePath: string;
    fileName: string;
};
interface SmoothingConfig {
    [key: string]: {
        sampleAlgorithm: string;
        sampleWeight: number;
    };
};

export class LossShowInfo {
    renderChart: boolean = false;
    showList: { [key: string]: string[]; } = {};
    private showListCopy: { [key: string]: string[]; } = {};
    tagAggregationShowList: CheckListItem[] = [];
    fileListChange: number = 0;
    stepGetData: number = 5;
    newFileList: NewFileResponseBody['data'] = [];
    private fileList: FileList = {};
    private hasBaseData: string[] = [];
    isTagPolymerize: boolean = false;
    smoothingConfigList: SmoothingConfig = {};
    showTagList: string[] = [];

    constructor() {
        makeAutoObservable(this);
    };

    private getIsChecked(tag: string, file: { name: string; path: string }) {
        if (!this.hasBaseData.includes(tag)) {
            this.hasBaseData.push(tag);
            this.showList[tag] = [file.path];
            this.showListCopy[tag] = [file.path];
            if (this.showTagList.length < 3) {
                this.showTagList.push(tag);
            }
            return true;
        }
        return false;
    };

    addFile(fileList: FileItem[]) {
        fileList.forEach((item: FileItem) => {
            if (!Object.keys(this.fileList).includes(item.tag)) {
                this.fileList[item.tag] = {};
                this.showList[item.tag] = [];
                this.smoothingConfigList[item.tag] = { sampleAlgorithm: '', sampleWeight: 0 };
            }
            item.fileList.forEach((file: { name: string; path: string }) => {
                if (!Object.keys(this.fileList[item.tag]).includes(file.path)) {
                    this.fileList[item.tag][file.path] = {
                        name: file.name,
                        data: [],
                        offset: 0,
                        isChecked: this.getIsChecked(item.tag, file),
                        smoothingData: [],
                        sampleOffset: 0,
                    };
                }
            });
        });
        this.fileListChange += 1;
    };

    addData(dataList: ChartsDataResponseBody['data']) {
        dataList.forEach(item => {
            this.fileList[item.tag][item.file].data.push(...item.points);
            this.fileList[item.tag][item.file].offset = this.fileList[item.tag][item.file].data.length;

            this.fileList[item.tag][item.file].smoothingData.push(...(item.sampledPoints ?? []));
            this.fileList[item.tag][item.file].sampleOffset = this.fileList[item.tag][item.file].smoothingData.length;
        });
        this.renderChart = !this.renderChart;
    };

    checkedListFormat(value: string[]) {
        const checkedList: { [key: string]: string[] } = {};
        value.forEach(item => {
            const [tag, file] = item.split(':');
            if (checkedList[tag] === undefined) {
                checkedList[tag] = [];
            }
            checkedList[tag].push(file);
        });
        return checkedList;
    };

    modifyCheck(value: { tag: string, fileList: string[] } | CheckListItem[]) {
        if (Array.isArray(value)) {
            this.tagAggregationShowList = value;
            const checkFilePathList = value.map(item => item.filePath);
            this.modifyTagAggregationCheck(this.checkedListFormat(checkFilePathList));
        } else {
            this.modifyOneCheck(value);
        }
    };

    private modifyOneCheck({ tag, fileList }: { tag: string, fileList: string[] }) {
        Object.keys(this.fileList[tag]).forEach(filePath => {
            if (fileList.includes(filePath)) {
                this.fileList[tag][filePath].isChecked = true;
            } else {
                this.fileList[tag][filePath].isChecked = false;
            }
            this.showList[tag] = fileList;
        });
    };

    private modifyTagAggregationCheck(value: { [key: string]: string[] }) {
        Object.keys(this.fileList).forEach(tag => {
            Object.keys(this.fileList[tag]).forEach(filePath => {
                this.fileList[tag][filePath].isChecked = false;
            });
            this.showList[tag] = [];
        });
        Object.keys(value).forEach(tag => {
            this.modifyOneCheck({ tag, fileList: value[tag] });
        });
    };

    modifyStepGetData(value: number) {
        this.stepGetData = value;
    };

    modifyNewFileList(value: NewFileResponseBody['data']) {
        this.newFileList = value;
    };

    modifyIsTagPolymerize(value: boolean) {
        const copyMid = JSON.parse(JSON.stringify(this.showList));
        this.modifyTagAggregationCheck((this.showListCopy));
        this.showListCopy = copyMid;
        this.isTagPolymerize = value;
    };

    modifySmoothingConfig(tag: string | string[], value: { sampleAlgorithm: string, sampleWeight: number }) {
        const tagList = typeof tag === 'string' ? [tag] : tag;
        tagList.forEach(tag => {
            this.smoothingConfigList[tag].sampleAlgorithm = value.sampleAlgorithm;
            this.smoothingConfigList[tag].sampleWeight = value.sampleWeight;
            Object.keys(this.fileList[tag]).forEach(filePath => {
                this.fileList[tag][filePath].smoothingData = [];
                this.fileList[tag][filePath].sampleOffset = 0;
            });
        });
    };

    modifyShowTagList(tags: string[]) {
        this.showTagList = tags;
    };

    getFileListByModel(model: string) {
        const res: FileListResult[] = [];
        Object.keys(this.fileList).forEach((tag: string) => {
            const file = Object.keys(this.fileList[tag]).map((filePath: string) => ({
                filePath,
                fileName: this.fileList[tag][filePath].name,
                value: this.fileList[tag][filePath][model],
            }));
            res.push({
                tag,
                file,
            });
        });
        return res;
    };

    getDataByTag(tag: string | string[]) {
        if (typeof tag === 'string') {
            return this.getDataByOneTag(tag);
        } else {
            return this.getDataByTagList(tag);
        }
    };

    private getDataByOneTag(tag: string) {
        const data: { [key: string]: DataItem[] } = {};
        const smoothingData: { [key: string]: DataItem[] } = {};
        if (!this.fileList[tag]) {
            return { data, columns: [], showList: [], smoothingData, smoothingColumns: [] };
        }

        Object.keys(this.fileList[tag]).forEach((file: string) => {
            const { sampleAlgorithm, sampleWeight } = this.smoothingConfigList[tag];
            if (this.fileList[tag][file].isChecked) {
                data[this.fileList[tag][file].name] = this.fileList[tag][file].data;
                smoothingData[`${this.fileList[tag][file].name}_smoothing`] =
                    (sampleAlgorithm === '' || sampleWeight === 0)
                        ? this.fileList[tag][file].data : this.fileList[tag][file].smoothingData;
            }
        });

        const columns: string[] = this.showList[tag].map(filePath => this.fileList[tag][filePath].name);
        const smoothingColumns: string[] = columns.map(item => `${item}_smoothing`);
        const showList: string[] = [...this.showList[tag]];

        return { data, columns, showList, smoothingData, smoothingColumns };
    };

    private getDataByTagList(tags: string[]) {
        const data: { [key: string]: DataItem[] } = {};
        const smoothingData: { [key: string]: DataItem[] } = {};
        const columns: string[] = [];
        const showList: string[] = [];
        tags.forEach(tag => {
            const { data: oneTagData, smoothingData: oneTagSmoothingData } = this.getDataByOneTag(tag);
            Object.keys(oneTagData).forEach(key => {
                data[`${tag}:${key}`] = oneTagData[key];
                smoothingData[`${tag}:${key}_smoothing`] = oneTagSmoothingData[`${key}_smoothing`];
            });
        });
        this.tagAggregationShowList.forEach(item => {
            columns.push(item.fileName);
            showList.push(item.filePath);
        });
        const smoothingColumns: string[] = columns.map(item => `${item}_smoothing`);

        return { data, columns, showList, smoothingData, smoothingColumns };
    };

    getGraphList() {
        const graphList: ChartsDataRequestParams['graphList'] = [];
        Object.keys(this.fileList).forEach((tag: string) => {
            if (this.showTagList.includes(tag)) {
                const smoothingConfig = this.smoothingConfigList[tag];
                Object.keys(this.fileList[tag]).forEach((file: string) => {
                    if (this.fileList[tag][file].isChecked) {
                        graphList.push({
                            ...smoothingConfig,
                            tag,
                            file,
                            offset: this.fileList[tag][file].offset,
                            sampleOffset: this.fileList[tag][file].sampleOffset,
                        });
                    }
                });
            }
        });
        return graphList;
    };

    getTagList() {
        return Object.keys(this.fileList);
    };

    getSmothingConfig(tag: string | string[]) {
        return typeof tag === 'string'
            ? { ...this.smoothingConfigList[tag] }
            : { ...this.smoothingConfigList[tag[0]] };
    };

    getShowList(tag: string | string[]) {
        if (typeof tag === 'string') {
            return this.showList[tag];
        } else {
            const showFileList: string[] = [];
            tag.forEach(item => showFileList.push(...this.showList[item]));
            return showFileList;
        }
    };

    clearFileList() {
        this.renderChart = false;
        this.fileList = {};
        this.showList = {};
        this.hasBaseData = [];
        this.stepGetData = 5;
        this.newFileList = [];
        this.fileListChange += 1;
        this.isTagPolymerize = false;
        this.showListCopy = {};
        this.tagAggregationShowList = [];
        this.smoothingConfigList = {};
        this.showTagList = [];
    };
};
