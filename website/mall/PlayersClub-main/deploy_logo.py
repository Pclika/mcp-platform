import paramiko

ssh = paramiko.SSHClient()
ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
ssh.connect('47.253.237.224', username='root', password='Bai9986650.', timeout=20)
sftp = ssh.open_sftp()

content = open('sylius-theme/templates/shared/layout/base/header/content.html.twig', 'r', encoding='utf-8').read()
for pre in ['/www/wwwroot/mall_pclika/templates/bundles/SyliusShopBundle',
            '/www/wwwroot/mall_pclika/src/Sylius/Bundle/ShopBundle/templates']:
    rel = 'shared/layout/base/header/content.html.twig'
    with sftp.open(f'{pre}/{rel}', 'w') as f:
        f.write(content)

sftp.close()

stdin, stdout, stderr = ssh.exec_command('cd /www/wwwroot/mall_pclika && php -d memory_limit=512M bin/console cache:clear --env=prod 2>&1 | tail -3', timeout=60)
print(stdout.read().decode())
ssh.exec_command('chown -R www:www /www/wwwroot/mall_pclika/var/cache/')

stdin, out, err = ssh.exec_command('curl -sL -o /dev/null -w "%{http_code}" -H "Host: mall.pclika.com" http://localhost/', timeout=10)
print('HTTP:', out.read().decode().strip())

stdin, out, err = ssh.exec_command('curl -sL -H "Host: mall.pclika.com" http://localhost/ 2>&1 | grep -c Fatal', timeout=10)
print('Errors:', out.read().decode().strip())

stdin, out, err = ssh.exec_command('curl -sL -H "Host: mall.pclika.com" http://localhost/ 2>&1 | grep -c "viewBox"', timeout=10)
print('SVG logo:', out.read().decode().strip())

ssh.close()
