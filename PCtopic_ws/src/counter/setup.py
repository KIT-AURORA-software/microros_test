from setuptools import find_packages, setup

package_name = 'counter'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='ma',
    maintainer_email='matsuo.shuntaro4981@mail.kyutech.jp',
    description='TODO: Package description',
    license='TODO: License declaration',
    extras_require={
        'test': [
            'pytest',
        ],
    },
    entry_points={
        'console_scripts': [
            'number_pub = counter.number_pub:main',
            'number_sub = counter.number_sub:main',
        ],
    },
)
