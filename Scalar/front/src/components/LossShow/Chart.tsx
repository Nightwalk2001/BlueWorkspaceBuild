/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import React, { useEffect, useRef, useState } from 'react';
import { observer } from 'mobx-react';
import {
    Form,
    Button,
    Table,
    Checkbox,
    type TablePaginationConfig,
    type TableProps,
    FormInstance,
} from 'antd';
import Chart from '../Echarts';
import type { CheckListItem, FileListResult, LossShowInfo } from '@/entity/lossShow';
import { downloadCsv, keepDecimals, notZero, getChartColor, type HeaderType } from '@/utils/common';
import type { ECBasicOption } from 'echarts/types/dist/shared';
import { Resizer } from '../common/Resizer';
import { useTranslation } from 'react-i18next';
import { Smoothing } from './Smoothing';
import { TFunction } from 'i18next';
import eventBus from '@/eventBus';

const creatDataChart = (columns: string[] = [], dataSource: Array<{ [key: string]: number | null }> = [],
    dataSourceourceSmoothing: Array<{ [key: string]: number | null }> = [], smoothingColumns: string[] = []): ECBasicOption => {
    const source = dataSource.map((item, index) => {
        return { ...item, ...dataSourceourceSmoothing[index] };
    });

    const dimensions: string[] = [];
    columns.forEach((item, index) => {
        dimensions.push(item, smoothingColumns[index]);
    });

    return {
        animation: false,
        tooltip: {
            trigger: 'axis',
            className: 'chartTooltip',
            axisPointer: {
                type: 'cross',
            },
            formatter: (seriesList: any[]) => chartTooltipFormat(seriesList, dimensions),
        },
        xAxis: {
            type: 'category',
        },
        yAxis: {
        },
        series: dimensions.map((_item, index) => ({
            symbolSize: 5,
            type: 'line',
            animation: false,
            progressive: 0,
            showSymbol: false,
            lineStyle: {
                color: getChartColor(index),
            },
            itemStyle: {
                color: getChartColor(index),
            },
        })),
        dataset: {
            dimensions: ['step', ...dimensions],
            source,
        },
        legend: {
            show: false,
            textStyle: {
                color: 'rgb(141, 152, 170)',
            },
            data: [...columns],
        },
    };
};

const chartTooltipFormat = (seriesList: any[], dimensions: string[]) => {
    if (seriesList.length < 1) {
        return;
    }
    const div = document.createElement('div');
    div.className = 'tooltip';
    div.append(`Stpe: ${seriesList[0].data.step}`);
    const tooltipItem = document.createElement('div');
    tooltipItem.className = 'tooltipItem';

    for (let i = 0; i < seriesList.length; i += 2) {
        const seriesItem = seriesList[i];
        const circle = document.createElement('div');
        circle.className = 'circle';
        circle.setAttribute('style', `background-color:${seriesList[i + 1].color};`);
        const keyDom = document.createElement('div');
        keyDom.className = 'value';
        keyDom.append(`value: ${seriesItem.data[dimensions[i]]}`);
        const valueDom = document.createElement('div');
        valueDom.className = 'smoothing';
        valueDom.append(`smoothing: ${seriesItem.data[dimensions[i + 1]]}`);

        tooltipItem.appendChild(circle);
        tooltipItem.appendChild(keyDom);
        tooltipItem.appendChild(valueDom);
    }
    div.appendChild(tooltipItem);

    return div;
};

const creatComparisonChart = (source: Array<{ [key: string]: number | null }> = [], t: TFunction): ECBasicOption => {

    return {
        animation: false,
        tooltip: {
            trigger: 'axis',
            axisPointer: {
                type: 'cross',
            },
        },
        xAxis: {
            type: 'category',
        },
        yAxis: [
            {
                type: 'value',
            },
            {
                type: 'value',
                position: 'left',
                axisLabel: {
                    formatter: '{value} %',
                }
            },
        ],
        series: ['comparisonNormal', 'comparisonAbsolute', 'comparisonRelative'].map(name => ({
            name: t(name),
            symbolSize: 5,
            type: 'line',
            animation: false,
            progressive: 0,
            yAxisIndex: 0,
            showSymbol: false,
        })),
        dataset: {
            dimensions: ['step', 'Comparison Normal', 'Comparison Absolute', 'Comparison Relative'],
            source,
        },
        legend: {
            textStyle: {
                color: 'rgb(141, 152, 170)',
            },
            selectedMode: 'single', // 每次打开一个图例
            selected: {},
        },
    };
};

const getTabelData = (data: { [key: string]: Array<{ step: number; value: number }> }) => {
    const tableDataSource: Array<{ [key: string]: number | null }> = [];
    const keys = Object.keys(data);

    if (keys.length === 0) {
        return { tableDataSource, len: { max: -1, min: -1 } };
    }
    const len = keys.length < 2
        ? data[keys[0]].length : Math.max(data[keys[0]].length, data[keys[1]].length);
    for (let i = 0; i < len; i++) {
        tableDataSource.push({ step: i, key: i });
    }
    for (let i = 0; i < tableDataSource.length; i++) {
        for (let j = 0; j < keys.length; j++) {
            tableDataSource[i][keys[j]] = data[keys[j]][i] ? data[keys[j]][i].value : null;
        }
    }

    return {
        tableDataSource, len: {
            max: len,
            min: keys.length < 2
                ? -1 : Math.min(data[keys[0]].length, data[keys[1]].length),
        },
    };
};

interface ChartDataLenInfo {
    [key: string]: {
        max: number;
        min: number;
    };
    dataLen: {
        max: number;
        min: number;
    };
    smoothingDataLen: {
        max: number;
        min: number;
    };
};
const DataChart = observer(({ lossShowInfo, tag, isExpand }:
    { lossShowInfo: LossShowInfo; tag: string | string[]; isExpand: boolean }): JSX.Element => {
    const { t } = useTranslation('lossShow');
    const [columns, setColumns] = useState<TableProps<object>['columns']>([]);
    const [dataNum, setDataNum] = useState<ChartDataLenInfo>({ dataLen: { max: -1, min: -1 }, smoothingDataLen: { max: -1, min: -1 } });
    const [showFileList, setShowFileList] = useState<string[]>([]);
    const [tableData, setTableData] = useState<Array<{ [key: string]: number | null }>>();
    const [chartOption, setChartOption] = useState({});
    const [smoothingData, setSmoothingData] = useState<Array<{ [key: string]: number | null }>>([]);
    const beforeRef = useRef<HTMLDivElement>(null);
    const [resizeChartFlag, setResizeChartFlag] = useState(0);
    const pagination: TablePaginationConfig = {
        simple: true,
        align: 'center',
        defaultCurrent: 1,
        defaultPageSize: 20,
        showSizeChanger: true,
        showTotal: (total) => t('dataSize', { total }),
        locale: {
            items_per_page: t('itemsPerPage'),
            prev_page: t('prevPage'),
            next_page: t('nextPage'),
        },
    };
    const chartHeight = {
        large: 890,
        small: 420,
    };

    const resizeChart = () => {
        setResizeChartFlag(Math.random());
    };

    const getColumns = (data: string[]) => {
        const res = data.map(item => ({
            title: item,
            dataIndex: item,
            key: item,
            ellipsis: true,
            width: 200,
        }));
        res.unshift({
            title: 'Step',
            dataIndex: 'step',
            key: 'step',
            ellipsis: true,
            width: 50,
        });
        return res;
    };
    const isRefreshChart = (showList: string[], len: ChartDataLenInfo, tableDataSourceSmoothing: Array<{ [key: string]: number | null }>) => {
        const dataListChange = showFileList.toString() !== showList.toString();
        const dataNumChange = Object.keys(dataNum).map(key => {
            return dataNum[key].min !== len[key].min || dataNum[key].max !== len[key].max;
        }).some(item => (item === true));
        const smoothingDataChange = JSON.stringify(smoothingData) !== JSON.stringify(tableDataSourceSmoothing);

        return dataListChange || dataNumChange || smoothingDataChange;
    };
    const init = () => {
        const { data, columns, showList, smoothingData, smoothingColumns } = lossShowInfo.getDataByTag(tag);
        const { tableDataSource, len: dataLen } = getTabelData(data);
        const { tableDataSource: tableDataSourceSmoothing, len: smoothingDataLen } = getTabelData(smoothingData);

        if (isRefreshChart(showList, { dataLen, smoothingDataLen }, tableDataSourceSmoothing.slice(0, 50))) {
            setChartOption(creatDataChart(columns, tableDataSource, tableDataSourceSmoothing, smoothingColumns));
            setDataNum({ dataLen, smoothingDataLen });
            setSmoothingData(tableDataSourceSmoothing.slice(0, 50));
            setColumns(getColumns(columns));
            setTableData(tableDataSource);
            setShowFileList(showList);
        }
    };

    const exportFile = (tag: string | string[]): void => {
        const fileName = `${typeof tag === 'string' ? tag : 'tagAggregation'}_${Date.now()}.csv`;
        downloadCsv(columns?.map(item => ({ key: item.key, title: item.title })) as HeaderType[], tableData, fileName);
    };

    useEffect(() => {
        init();
    }, [lossShowInfo.getShowList(tag), lossShowInfo.renderChart]);

    useEffect(() => {
        setResizeChartFlag(Math.random());
    }, [t]);

    return <div className="chartContainer" style={{ height: isExpand && columns?.length !== 3 ? chartHeight.large : chartHeight.small }}>
        <div className="fileList" ref={beforeRef} style={{ width: 240 }}>
            <Smoothing lossShowInfo={lossShowInfo} tag={tag} />
            {
                Array.isArray(tag)
                    ? <FileListTagAggregation lossShowInfo={lossShowInfo} tags={tag} />
                    : <FileList lossShowInfo={lossShowInfo} tag={tag} />
            }
        </div>
        <Resizer beforeRef={beforeRef} range={{ min: 240, max: 800 }} callback={resizeChart}
            style={{ backgroundColor: 'var(--border-color-primary)', marginBottom: 30 }} />
        <div className="chart">
            {
                showFileList.length > 0 && <Chart option={chartOption} opts={{ replaceMerge: ['series'] }} toolbox={{ show: true, right: '10%', top: '20px' }}
                    style={{ height: isExpand && columns?.length !== 3 ? chartHeight.large : chartHeight.small }} doResize={resizeChartFlag} />
            }
        </div>
        <div className="table">
            {
                showFileList.length > 0 && <>
                    <Table pagination={pagination} columns={columns} dataSource={tableData}
                        size="small" scroll={{ y: isExpand && columns?.length !== 3 ? 780 : 310 }} />
                    <Button type="primary" className="downloadBtn" size="small" onClick={() => exportFile(tag)}>
                        {t('exportFile')}
                    </Button>
                </>
            }
        </div>
    </div>;
});

const ComparisonChart = ({ lossShowInfo, tag }: { lossShowInfo: LossShowInfo; tag: string | string[] }): JSX.Element => {
    const { t } = useTranslation('lossShow');
    const [chartOption, setChartOption] = useState({});
    const [dataNum, setDataNum] = useState(-1);
    const [showFileList, setShowFileList] = useState<string[]>([]);
    const [columns, setColumns] = useState<TableProps<object>['columns']>([]);
    const [tableData, setTableData] = useState<Array<{ [key: string]: number | null }>>();
    const [dataInfo, setDataInfo] = useState({} as { baseLine: string; comparison: string; });
    const beforeRef = useRef<HTMLDivElement>(null);
    const [resizeChartFlag, setResizeChartFlag] = useState(0);
    const pagination: TablePaginationConfig = {
        simple: true,
        align: 'center',
        defaultCurrent: 1,
        defaultPageSize: 20,
        showSizeChanger: true,
        showTotal: (total) => t('dataSize', { total }),
        locale: {
            items_per_page: t('itemsPerPage'),
            prev_page: t('prevPage'),
            next_page: t('nextPage'),
        },
    };

    const getColumns = () => {
        const columns = [
            { key: 'Comparison Normal', name: 'comparisonNormal' },
            { key: 'Comparison Absolute', name: 'comparisonAbsolute' },
            { key: 'Comparison Relative', name: 'comparisonRelative' },
        ];
        const res = columns.map(item => ({
            title: t(item.name),
            dataIndex: item.key,
            key: item.key,
            ellipsis: true,
            width: 120,
        }));
        res[2].width = 135;
        res.unshift({
            title: 'Step',
            dataIndex: 'step',
            key: 'step',
            ellipsis: true,
            width: 35,
        });
        return res;
    };

    const getData = (fileList: string[], data: { [key: string]: Array<{ step: number; value: number }> }) => {
        const dataSource: Array<{ [key: string]: number }> = [];
        const keys = Object.keys(data);
        if (keys.length < 2) {
            return { dataSource, len: -1 };
        }
        const len = Math.min(data[keys[0]].length, data[keys[1]].length);
        for (let i = 0; i < len; i++) {
            dataSource.push({ step: i, key: i });
        }
        for (let i = 0; i < len; i++) {
            const baseData = data[fileList[1]][i] ? data[fileList[1]][i].value : 0;
            const comparisonData = data[fileList[0]][i] ? data[fileList[0]][i].value : 1;
            if (typeof baseData === 'string' && typeof comparisonData === 'string') {
                continue;
            }
            const dif = baseData - comparisonData;
            dataSource[i]['Comparison Normal'] = dif;
            dataSource[i]['Comparison Absolute'] = Math.abs(dif);
            dataSource[i]['Comparison Relative'] = keepDecimals(Math.abs(dif) / notZero(comparisonData) * 100);
        }
        return { dataSource, len };
    };

    const exportFile = (tag: string | string[]): void => {
        const fileName = `${typeof tag === 'string' ? tag : 'tagAggregation'}_comparison_${Date.now()}.csv`;
        downloadCsv(columns?.map(item => ({ key: item.key, title: item.title })) as HeaderType[], tableData as any[], fileName);
    };

    const init = () => {
        const { data, columns, showList } = lossShowInfo.getDataByTag(tag);
        const { dataSource, len } = getData(columns, data);
        if (dataNum >= len && showFileList.toString() === showList.toString()) {
            return;
        }
        setShowFileList(showList);
        setTableData(dataSource);
        setColumns(getColumns());
        setDataInfo({ baseLine: columns[0], comparison: columns[1] });
        setDataNum(len);
        setChartOption(creatComparisonChart(dataSource, t));
    };

    const resizeChart = () => {
        setResizeChartFlag(Math.random());
    };

    useEffect(() => {
        setColumns(getColumns());
        setChartOption(creatComparisonChart(tableData, t));
    }, [t]);

    useEffect(() => {
        init();
    }, [lossShowInfo.getDataByTag(tag).showList]);
    return <div className="comparisonChart">
        <div className="title">{t('dataComparison')}</div>
        <div className="comparisonChartContainer">
            <div className="dataInfo" ref={beforeRef} style={{ width: 240 }}>
                <div className="item">
                    <div className="type">{t('baselineData')}：</div>
                    <div className="fileName" title={dataInfo.baseLine}>{dataInfo.baseLine}</div>
                </div>
                <div className="item">
                    <div className="type">{t('comparativeData')}：</div>
                    <div className="fileName" title={dataInfo.comparison}>{dataInfo.comparison}</div>
                </div>
            </div>
            <Resizer beforeRef={beforeRef} range={{ min: 240, max: 800 }} callback={resizeChart}
                style={{ backgroundColor: 'var(--border-color-primary)', marginbottom: 30 }} />
            <div className="chart">
                <Chart option={chartOption} toolbox={{ show: true, right: '10%', top: '20px' }}
                    style={{ height: 400 }} doResize={resizeChartFlag} />
            </div>
            <div className="table">
                <Table pagination={pagination} columns={columns} dataSource={tableData}
                    size="small" scroll={{ y: 310 }} />
                <Button type="primary" className="downloadBtn" size="small" onClick={() => exportFile(tag)}>
                    {t('exportFile')}
                </Button>
            </div>
        </div>
    </div>;
};

const getCkeckedClass = (value: string, form: FormInstance<any>, tag: string) => {
    const checkedList = form.getFieldValue(tag);
    const index = checkedList.findIndex(((item: string) => item.includes(value)));
    let className = '';
    switch (index) {
        case 0:
            className = 'checkedFirst';
            break;
        case 1:
            className = 'checkedSecond';
            break;
        default:
            break;
    }
    return className;
};

const FileList = ({ lossShowInfo, tag }:
    { lossShowInfo: LossShowInfo; tag: string }): JSX.Element => {
    const [selectList, setSelectList] = useState([] as FileListResult['file']);
    const [form] = Form.useForm();

    const getChecked = (file: FileListResult['file']) => {
        const res: string[] = [];
        file.forEach(item => {
            if (item.value) {
                res.push(item.filePath);
            }
        });
        return res;
    };

    const onChange = (tag: string, value: string[]): void => {
        if (value.length > 2) {
            value.shift();
        }
        lossShowInfo.modifyCheck({ tag, fileList: value });
        eventBus.emit('updataChartData');
    };

    const init = () => {
        const fileList = lossShowInfo.getFileListByModel('isChecked').find(item => item.tag === tag);
        if (fileList === undefined) {
            return;
        }
        setSelectList(fileList.file);
        form.setFieldValue(tag, getChecked(fileList.file));
    };

    useEffect(() => {
        init();
    }, [lossShowInfo.fileListChange]);

    return <Form form={form}>
        <Form.Item name={tag} key={tag} style={{ marginLeft: 10 }}>
            <Checkbox.Group className="checkboxGroup"
                onChange={(value) => onChange(tag, value)} >
                {
                    selectList.map(item => (
                        <div className="checkboxItem" title={item.filePath} key={item.filePath}>
                            <Checkbox className={getCkeckedClass(item.filePath, form, tag)}
                                value={item.filePath}>{item.fileName}</Checkbox>
                        </div>
                    ))
                }
            </Checkbox.Group>
        </Form.Item>
    </Form>;
};

const FileListTagAggregation = ({ lossShowInfo, tags }: { lossShowInfo: LossShowInfo; tags: string[] }): JSX.Element => {
    const [form] = Form.useForm();
    const [tagAggregationCheck, setTagAggregationCheck] = useState([] as Array<{ tag: string; filePath: string; fileName: string }>);

    const onChange = (checkedValue: string[]): void => {
        if (checkedValue.length > 2) {
            checkedValue.shift();
        }
        lossShowInfo.modifyCheck(checkedValue.map(item => JSON.parse(item)));
        eventBus.emit('updataChartData');
    };

    const init = () => {
        const selectList: CheckListItem[] = [];
        const checkedList: CheckListItem[] = [];

        const fileList = lossShowInfo.getFileListByModel('isChecked');

        tags.forEach(tag => {
            fileList.find(item => item.tag === tag)?.file.forEach(file => {
                selectList.push({
                    tag: tag,
                    filePath: `${tag}:${file.filePath}`,
                    fileName: `${tag}:${file.fileName}`,
                });
                if (file.value) {
                    checkedList.push({
                        tag: tag,
                        filePath: `${tag}:${file.filePath}`,
                        fileName: `${tag}:${file.fileName}`,
                    });
                }
            });
        });

        setTagAggregationCheck(selectList);
        lossShowInfo.modifyCheck(checkedList.slice(0, 2));
        form.setFieldValue('tagAggregationCheckbox', checkedList.slice(0, 2).map(item => JSON.stringify(item)));
    };

    useEffect(() => {
        init();
    }, [tags]);

    return <Form form={form}>
        <Form.Item name="tagAggregationCheckbox" key="tagAggregationCheckbox" style={{ marginLeft: 10 }}>
            <Checkbox.Group className="checkboxGroup"
                onChange={onChange} >
                {
                    tagAggregationCheck.map(item => (
                        <div className="checkboxItem" title={item.filePath} key={item.filePath}>
                            <Checkbox className={getCkeckedClass(item.filePath, form, 'tagAggregationCheckbox')}
                                value={JSON.stringify(item)} >{item.fileName}</Checkbox>
                        </div>
                    ))
                }
            </Checkbox.Group>
        </Form.Item>
    </Form>;
};

export const ChartsContainer = observer(({ lossShowInfo }: { lossShowInfo: LossShowInfo }) => {
    const { t } = useTranslation('lossShow');
    const tags = lossShowInfo.showTagList;

    const isShowComparisonChart = (tag: string): boolean => {
        const { showList } = lossShowInfo.getDataByTag(tag);
        return showList.length === 2;
    };

    return <>
        {
            tags.length > 0 ? tags.map(tag => (<div className="chartsContainer" key={tag}>
                <div className="title">{tag}</div>
                <DataChart lossShowInfo={lossShowInfo} tag={tag} isExpand={tags.length === 1} />
                {isShowComparisonChart(tag) && <ComparisonChart lossShowInfo={lossShowInfo} tag={tag} />}
            </div>)) : <div className="noData">{t('noData')}</div>
        }
    </>;
});

export const ChartsContainerTagAggregation = observer(({ lossShowInfo }: { lossShowInfo: LossShowInfo }) => {
    const { t } = useTranslation('lossShow');

    const tags = lossShowInfo.showTagList;
    const isShowComparisonChart = (): boolean => {
        const { showList } = lossShowInfo.getDataByTag(tags);
        if (showList.length !== 2) {
            return false;
        }
        return true;
    };
    return <>
        {
            tags.length > 0 ? <div className="chartsContainer">
                <div className="title">{t('tagAggregation')}</div>
                <DataChart lossShowInfo={lossShowInfo} tag={tags} isExpand={true} />
                {isShowComparisonChart() && <ComparisonChart lossShowInfo={lossShowInfo} tag={tags} />}
            </div> : <div className="noData">{t('noData')}</div>
        }
    </>;
});
