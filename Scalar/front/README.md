# 开发模式
开发模式下会显示插件自身的文件导入，语言切换和主题切换，方便插件的单独开发调试
## 启动
```
npm run dev
```

## 打包
```
npm run buildDev
```
# 生产模式
生产模式下会隐藏插件自身的文件导入，语言切换和主题切换；改为接收事件来进行对应操作，具体事件见src/App.tsx文件
## 启动
```
npm run start
```

## 打包
```
npm run build
```