erDiagram
    学院 {
        string 学院名称 PK
        string 办公地点
        string 院长姓名
    }
    班级 {
        string 班级号 PK
        int 学生人数
    }
    学生 {
        string 学号 PK
        string 姓名
        date 出生日期
    }
    宿舍 {
        string 宿舍编号 PK
        string 地址
        int 可住人数
    }
    班主任 {
        string 职工号 PK
        string 姓名
        string 性别
    }
    班导师 {
        string 职工号 PK
        string 姓名
        string 性别
    }
    学院 ||--o{ 班级 : 包含
    班级 ||--o{ 学生 : 拥有
    宿舍 ||--o{ 学生 : 入住
    班主任 ||--|| 班级 : 负责
    班导师 ||--|| 班级 : 指导