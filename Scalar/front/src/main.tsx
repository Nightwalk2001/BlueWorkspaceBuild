/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import { createRoot } from 'react-dom/client';
import App from './App.tsx';
import './i18n';
import './assets/index.scss';
import { RootStoreContext } from './context/context';
import { store } from './store';

// 禁用右键以及F5、Ctrl+R刷新
document.oncontextmenu = (): boolean => false;
document.addEventListener('keydown', (e) => {
    const forbiddenComboKeys = ['f', 'p', 'g', 'j', 'r'];
    const forbiddenSingleKeys = ['F3', 'F5', 'F7'];
    const isCtrlCombo = (e.ctrlKey || e.metaKey) && forbiddenComboKeys.includes(e.key.toLowerCase());
    if (isCtrlCombo || forbiddenSingleKeys.includes(e.key)) {
        e.preventDefault();
    }
});

createRoot(document.getElementById('root') as HTMLElement).render(
    <RootStoreContext.Provider value={store}>
        <App />
    </RootStoreContext.Provider>
);
