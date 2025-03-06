/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import React, { useEffect, useRef } from 'react';
import * as echarts from 'echarts';
import type { ECBasicOption } from 'echarts/types/dist/shared';
import { useTranslation } from 'react-i18next';
import { TFunction } from 'i18next';

interface ChartStyle {
    width?: number | string;
    height?: number | string;
};
interface EChartOpts {
    notMerge?: boolean;
    replaceMerge?: string | string[];
    lazyUpdate?: boolean;
};
interface ToolboxOpts {
    show: boolean;
    right?: string;
    top?: string;
};

const getToolbox = (toolbox: ToolboxOpts, cInstance: React.MutableRefObject<echarts.ECharts | undefined>,
    t: TFunction) => {
    return {
        feature: {
            dataZoom: {
                yAxisIndex: 'none',
                title: {
                    zoom: t('zoom'),
                    back: t('zoomReset'),
                },
            },
            myRestore: {
                title: t('restore'),
                icon: 'M3.8,33.4 M47,18.9h9.8V8.7 M56.3,20.1 C52.1,9,40.5,0.6,26.8,2.1C12.6,3.7,1.6,16.2,2.1,30.6 M13,41.1H3.1v10.2 M3.7,39.9c4.2,11.1,15.8,19.5,29.5,18 c14.2-1.6,25.2-14.1,24.7-28.5 ',
                onclick: function (echart: any) {
                    const options = echart.getOption();
                    cInstance.current?.clear();
                    cInstance.current?.setOption(options);
                },
            },
        },
        right: toolbox.right,
        top: toolbox.top,
    };
};

const MyChart = ({ option, opts = {}, toolbox = { show: true }, style, doResize }:
    { option: ECBasicOption; opts?: EChartOpts; toolbox?: ToolboxOpts; style?: ChartStyle; doResize?: number }): JSX.Element => {
    const { t } = useTranslation('lossShow');
    const cRef = useRef<HTMLDivElement>(null);
    const cInstance = useRef<echarts.EChartsType>();

    const resize = () => {
        cInstance.current?.resize({
            animation: { duration: 0 },
        });
    };

    const init = () => {
        if (cRef.current === null) {
            return;
        }
        cInstance.current = echarts.getInstanceByDom(cRef.current);
        if (cInstance.current === undefined) {
            cInstance.current = echarts.init(cRef.current, undefined, {
                renderer: 'canvas',
            });
        }
        if (option) {
            if (toolbox.show) {
                option.toolbox = getToolbox(toolbox, cInstance, t);
            }
            cInstance.current?.setOption(option, opts);
        }
    };
    
    useEffect(() => {
        init();
    }, [cRef, option]);

    useEffect(() => {
        window.addEventListener('resize', resize);

        return () => {
            window.removeEventListener('resize', resize);
        };
    }, [option]);

    useEffect(() => {
        resize();
    }, [style?.height, doResize]);

    useEffect(() => {
        cInstance.current?.setOption({ toolbox: getToolbox(toolbox, cInstance, t), });
    }, [t]);

    return (
        <div ref={cRef} style={style} />
    );
};

export default MyChart;
