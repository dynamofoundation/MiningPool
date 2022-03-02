# MiningPool

Compiling:

To compile for windows, open in VS2019 and build.  You will need curl and sqlite3.  These dependencies are most easily resolved with vcpkg.

To compile for linux:

```
apt-get install libsqlite3-dev

g++-11 -I. -std=gnu++11 -fpermissive *.cpp -lcurl -lsqlite3 -lpthread -o MiningPool
```

The standard installation requires 3 files:

MiningPool binary
settings.txt  (contains json string of settings)
webpack.dat   (contains packaged website for user interface)

Requirements:
1.  Dynamo fullnode with RPC enabled and sync to blockchain

2.  Create a wallet on the fullnode to hold mining proceeds and distribute.  Example:
```
dyn-cli -rpcconnect=127.0.0.1 -rpcuser=user -rpcpassword=123456 createwallet "mining"
```

3.  Create an address to save the mining proceeds in.  Example:
```
dyn-cli -rpcconnect=127.0.0.1 -rpcuser=user -rpcpassword=123456 getnewaddress
```
(note this address for the settings file later)

4.  Update dynamo.conf for the new wallet and restart fullnode.  Example:
```
wallet=<name of wallet created in step 2>
```
(this is not necessary but will make pool startup faster)

5.  Update settings.txt with your desired parameters.

6.  Start MiningPool binary.

Explanation of settings.txt:

```
{
	"clientListenPort" : 4567,        (This is the port miners will connect to)

	"rpcURL" : "http://192.168.1.193:6433/",     (RPC URL of your fullnode with the mining wallet)
	"rpcUser" : "user",
	"rpcPassword" : "123456",
	"rpcWallet" : "mining",           (Name of the wallet you created in step 2)

	"payoutWallet" : "dy1qzvx3yfrucqa2ntsw8e7dyzv6u6dl2c2wjvx5jy",      (Address to pay operator proceeds to.  Must be different than miningWallet)
	"miningWallet" : "dy1qpd28y8xxwv6ra80s3e9w5wmfhjgldayp576w8p",      (Address created in step 3.  This is where miners will mine to and payouts will be made from)
	"miningFeePercent" : 100,         (Fee to charge on each payout as a percent, to two decimals.  100 = 1%, 50 = 0.5%, 300 = 3%, etc)
	"payoutPeriod" : 3600,            (How often to run payouts, in seconds.  3600 = 1 hour)
	"minPayout" : 50000000,           (The min amount to pay out, in atoms.  Must be more than 10000)
	"payoutFee" : 10000,              (The payout fee to use for wallet transactions.  Should be minrelay, which is currently 10000)
	
	"httpPort" : 8086                 (The port number for the HTTP server user interface)
}
```
