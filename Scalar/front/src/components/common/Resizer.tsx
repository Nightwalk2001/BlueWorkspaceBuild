/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

import React, { useRef } from 'react';

export const Resizer = ({ beforeRef, range, style, callback }:
    { beforeRef: React.RefObject<HTMLDivElement>; range: { min: number; max: number }; style?: object; callback?: () => void }): JSX.Element => {
    const resizeRef = useRef<HTMLDivElement>(null);
    let isDrag = false;
    let offsetX: number;
    let oldWidth: number;

    const handleMouseDown = (event: React.MouseEvent<HTMLDivElement, MouseEvent>) => {
        event.preventDefault();
        if (beforeRef.current) {
            oldWidth = parseInt(beforeRef.current.style.width);
        }
        isDrag = true;
        offsetX = event.clientX;
        document.addEventListener('mousemove', handleMouseMove);
        document.addEventListener('mouseup', handleMouseUp);
    };

    const handleMouseMove = (event: MouseEvent) => {
        event.preventDefault();

        if (isDrag && resizeRef.current && beforeRef.current) {
            const moveLen = event.clientX - offsetX;
            const width = oldWidth + moveLen;
            if (width > range.min && width < range.max) {
                beforeRef.current.style.width = `${width}px`;
                if (callback) {
                    callback();
                }
            }
        }
    };

    const handleMouseUp = (event: MouseEvent) => {
        if (event !== undefined) {
            event.preventDefault();
        }
        isDrag = false;
        document.removeEventListener('mousemove', handleMouseMove);
        document.removeEventListener('mouseup', handleMouseUp);
    };

    return <div ref={resizeRef} onMouseDown={handleMouseDown}
        style={{ width: 3, backgroundColor: 'var(--text-color-primary)', cursor: 'w-resize', position: 'relative', zIndex: 10, ...style }}></div>;
};