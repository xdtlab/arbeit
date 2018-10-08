# Arbeit

Arbeit is a very simple Argon2i CPU miner for Daten!

## Usage

You should have `pydaten` installed on your system. See the instructions in the project's repo.

Run the commands below:
```bash
git clone --recurse-submodules https://github.com/xdtlab/arbeit.git
cd arbeit/arbeit
make
./solo --url        [Url of your mining endpoint] \
       --username   [Username of the endpoint] \
       --password   [Password of the endpoint] \
       --address    [Where the mined tokens are going to]
```
