-- 定义路径
setExecPath("Horizon/src/modules/scheduler/stuff/jumpshot")

-- 设置WASD可取消标志
wasdCancel()

-- 执行路径跳转
jump()

-- 等待24tick（约0.375秒，按1秒=64tick换算）
sleep(24)

-- 执行攻击动作
attack(）  -- 按下攻击
sleep(1)  -- 保持1tick
attack(-1) -- 释放攻击