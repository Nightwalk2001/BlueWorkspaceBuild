/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { observer } from 'mobx-react';
import { LossContainer } from './components/LossShow';
import { useRootStore } from './context/context';
import { useEffect, useState } from 'react';
import { setBaseURL } from './api/request';
import eventBus from './eventBus';
import './assets/App.scss';
import i18n from './i18n';
import { Button, Spin } from 'antd';
import { safeJSONParse } from './utils/common';

const App = observer(() => {
    const [hasURL, setHasURL] = useState(false);
    const [loading, setLoading] = useState(false);
    const { lossShowStore } = useRootStore();
    const lossShowInfo = lossShowStore.activeLossShow;
    const isProduction = import.meta.env.VITE_MODE === 'production';

    if (isProduction) {
        window.parent.postMessage({ event: 'pluginMounted' }, '*');

        window.onmessage = async (e) => {
            const { target, event, data, body } = typeof e.data === 'string' ? safeJSONParse(e.data) : e.data;
            if (target !== 'plugin') {
                return;
            }

            switch (event) {
                case 'wakeupPlugin':
                    setBaseURL(data.url);
                    setHasURL(true);
                    window.parent.postMessage({ event: 'getLanguage' }, '*');
                    window.parent.postMessage({ event: 'getTheme' }, '*');
                    break;
                case 'switchLanguage':
                    i18n.changeLanguage(body.lang);
                    break;
                case 'setTheme':
                    if (body.isDark) {
                        document.documentElement.classList.add('dark');
                        document.body.className = 'dark';
                    } else {
                        document.documentElement.classList.remove('dark');
                        document.body.className = 'light';
                    }
                    break;
                case 'remote/import':
                    eventBus.emit('importFile', body.dataSource.projectName);
                    break;
                case 'remote/remove':
                case 'remote/reset':
                    lossShowInfo?.clearFileList();
                    eventBus.emit('removeFile');
                    break;
                default:
                    break;
            }
        };
    }

    const [lang, setLang] = useState('enUS');
    const changeLang = () => {
        const langType = lang === 'enUS' ? 'zhCN' : 'enUS';
        setLang(langType);
        i18n.changeLanguage(langType);
    };

    const [theme, setTheme] = useState('dark');
    const changeTheme = () => {
        const themeType = theme === 'dark' ? 'light' : 'dark';
        setTheme(themeType);
        if (themeType === 'dark') {
            document.documentElement.classList.add('dark');
            document.body.className = 'dark';
        } else {
            document.documentElement.classList.remove('dark');
            document.body.className = 'light';
        }
    };

    useEffect(() => {
        eventBus.on('setLoading', setLoading);
        return () => {
            eventBus.off('setLoading', setLoading);
        };
    });
    return <>
        <Spin spinning={loading} fullscreen />
        {
            isProduction && (lossShowInfo !== undefined && hasURL ? (
                <div className="mainContainer" style={{ display: loading ? 'none' : 'block' }}>
                    <LossContainer lossShowInfo={lossShowInfo} />
                </div>
            ) : <></>)
        }
        {
            !isProduction && (lossShowInfo !== undefined ? (
                <div className="mainContainer">
                    <div style={{ position: 'absolute', zIndex: 10000, right: 100, top: 30 }}>
                        <Button type="primary" onClick={changeLang}>切换语言</Button>
                        <Button type="primary" onClick={changeTheme}>切换主题</Button>
                    </div>
                    <LossContainer lossShowInfo={lossShowInfo} />
                </div>
            ) : <></>)
        }
    </>;
});

export default App;
