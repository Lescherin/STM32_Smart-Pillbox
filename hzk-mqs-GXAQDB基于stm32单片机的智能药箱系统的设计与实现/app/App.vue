<script>
import DB from '@/utils/sqlite.js'; // 导入放在外部，正确

export default {
  onLaunch: async function() { // 声明为 async，允许内部使用 await
    async function initUserTable() {
      try {
        // 打开数据库
        await DB.openSqlite();
        console.log('数据库打开成功');
         
        // 创建用户表（使用 CREATE TABLE IF NOT EXISTS，避免重复创建报错）
        await DB.createTable(
          'user', 
          `"id" INTEGER PRIMARY KEY AUTOINCREMENT,
           "username" TEXT UNIQUE NOT NULL,
           "password" TEXT NOT NULL,
           "createdAt" TIMESTAMP DEFAULT CURRENT_TIMESTAMP`
        );
        
        // 验证表是否创建成功
        const tables = await DB.QueryAllTables();
        const userTableExists = tables.some(table => table.name === 'user');
        if (userTableExists) {
          console.log('用户表创建成功并已存在');
        } else {
          console.error('用户表创建失败：表未被创建');
        }
      } catch (e) {
        console.error('用户表初始化失败:', e.message || e);
      }
    }
    
    // 关键：调用初始化函数，并等待其完成
    await initUserTable();
    console.log('App Launch');
  },
  onShow: function() {
    console.log('App Show');
  },
  onHide: function() {
    console.log('App Hide');
  },
};
</script>

<style>
/* 全局公共样式 */
page {
  height: 100%;
  background-color: #f5f7fa;
}

/* 底部导航栏样式 */
.tabbar-placeholder {
  height: 100rpx; /* 与 tabBar 高度一致 */
}

/* 登录页面隐藏导航栏 */
.login-page .uni-tabbar {
  display: none;
}
</style>