from mininet.net import Mininet
from mininet.node import RemoteController

# 创建Mininet实例
net = Mininet(controller=RemoteController)

# 添加主机和交换机
h1 = net.addHost('h1')
h2 = net.addHost('h2')
s1 = net.addSwitch('s1')

# 添加链路
net.addLink(h1, s1)
net.addLink(h2, s1)

# 启动拓扑
net.start()

# 执行命令
h1.cmd('ls')

# 停止拓扑
net.stop()