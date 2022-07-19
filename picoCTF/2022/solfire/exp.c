/**
 * @brief C-based Helloworld BPF program
 */
#include <solana_sdk.h>

#define CLOCK 0
#define SYSTEM 1
#define BALANCES 2
#define VAULT 3
#define SOLFIRE 4
#define SOLVE 5
#define USER 6
#define SOLFIRE_SEED 0
#define BALANCES_SEED 1
#define VAULT_SEED 2
#define CREATE_OPCODE 0
#define DEPOSIT_OPCODE 1
#define WITHDRAW_OPCODE 2


typedef struct {
	uint32_t opcode;    // Set to 0 for handle_create
	uint8_t bump_seed;  // Used for signing (so just the bump byte), the seed doesn't actually need to get used, just be valid for the solfire program
} CreateAccountInput;

typedef struct {
    uint32_t opcode; // Set to 1 for handle_deposit
    uint32_t balance_index; // (MAX: 640) An index on the solfire owned account where the deposit will be added to
    uint32_t lamports; // (MIN: 1) The number of lamports to send
} DepositInput;

typedef struct {
    uint32_t opcode; // Set to 2 for handle_withdraw
    uint32_t balance_index; // (MAX: 640) An index on the solfire owned account where the withdraw will come from
    uint32_t lamports; // (MIN: 1) The number of lamports to send
    uint8_t vault_bump_seed; // A bump seed used for vault signing
} WithdrawInput;

#pragma pack(push, 1)
typedef struct {
    uint32_t opcode;
    uint64_t lamports;
    uint64_t space;
    SolPubkey owner;
} SystemCreateAccountInput;
#pragma pack(pop)


void system_create(SolParameters *params) {
    uint8_t seed_data[] = { 'A', params->data[BALANCES_SEED] };
    SolSignerSeed seed = {
        .addr = seed_data,
        .len = SOL_ARRAY_SIZE(seed_data),
    };

    const SolSignerSeeds signers_seeds[] = { 
        { &seed, 1 },
    };
    SolAccountInfo *infos = params->ka;
    SolAccountMeta metas[] = {
        { .pubkey = infos[USER].key, .is_writable = true, .is_signer = true },
        { .pubkey = infos[BALANCES].key, .is_writable = true, .is_signer = true }
    };

    SystemCreateAccountInput ix_data = {
        .opcode = 0,
        .lamports = 1,
        .space = 0,
        .owner = *infos[SOLFIRE].key
    };

    SolInstruction instruction = {
        .program_id = infos[SYSTEM].key,
        .accounts = metas,
        .account_len = SOL_ARRAY_SIZE(metas),
        .data = (uint8_t*)&ix_data,
        .data_len = sizeof(ix_data)
    };

    sol_invoke_signed(&instruction, infos, params->ka_num, signers_seeds, 1);
}

void create(SolParameters *params) {
    SolAccountInfo *infos = params->ka;
    SolAccountMeta metas[] = {
        { .pubkey = infos[CLOCK].key, .is_writable = false, .is_signer = false },
        { .pubkey = infos[SYSTEM].key, .is_writable = false, .is_signer = false },
        { .pubkey = infos[BALANCES].key, .is_writable = true, .is_signer = false },
        { .pubkey = infos[VAULT].key, .is_writable = false, .is_signer = false },
        { .pubkey = infos[USER].key, .is_writable = true, .is_signer = true }
    };

    CreateAccountInput ix_data = {
        .opcode = CREATE_OPCODE,
        .bump_seed = params->data[SOLFIRE_SEED]
    };

    SolInstruction instruction = {
        .program_id = infos[SOLFIRE].key,
        .accounts = metas,
        .account_len = SOL_ARRAY_SIZE(metas),
        .data = (uint8_t*)&ix_data,
        .data_len = sizeof(ix_data)
    };

    sol_invoke(&instruction, infos, params->ka_num);
}

void deposit(SolParameters *params, SolPubkey* recipient, SolPubkey* funder, uint32_t lamports) {
    SolAccountInfo *infos = params->ka;
    SolAccountMeta metas[] = {
        { .pubkey = infos[CLOCK].key, .is_writable = false, .is_signer = false },
        { .pubkey = infos[SYSTEM].key, .is_writable = false, .is_signer = false },
        { .pubkey = infos[BALANCES].key, .is_writable = true, .is_signer = false },
        { .pubkey = funder, .is_writable = true, .is_signer = true },
        { .pubkey = recipient, .is_writable = true, .is_signer = false }
    };

    DepositInput ix_data = {
        .opcode = DEPOSIT_OPCODE,
        .balance_index = 0,
        .lamports = lamports
    };

    SolInstruction instruction = {
        .program_id = infos[SOLFIRE].key,
        .accounts = metas,
        .account_len = SOL_ARRAY_SIZE(metas),
        .data = (uint8_t*)&ix_data,
        .data_len = sizeof(ix_data)
    };

    sol_invoke(&instruction, infos, params->ka_num);
}

void withdraw(SolParameters *params, SolPubkey* recipient, SolPubkey* funder, uint32_t balance_index, uint32_t lamports) {
    SolAccountInfo *infos = params->ka;
    SolAccountMeta metas[] = {
        { .pubkey = infos[CLOCK].key, .is_writable = false, .is_signer = false },
        { .pubkey = infos[SYSTEM].key, .is_writable = false, .is_signer = false },
        { .pubkey = infos[BALANCES].key, .is_writable = true, .is_signer = false },
        { .pubkey = recipient, .is_writable = true, .is_signer = true },
        { .pubkey = funder, .is_writable = true, .is_signer = false }
    };

    WithdrawInput ix_data = {
        .opcode = WITHDRAW_OPCODE,
        .balance_index = balance_index,
        .lamports = lamports,
        .vault_bump_seed = params->data[VAULT_SEED]
    };

    SolInstruction instruction = {
        .program_id = infos[SOLFIRE].key,
        .accounts = metas,
        .account_len = SOL_ARRAY_SIZE(metas),
        .data = (uint8_t*)&ix_data,
        .data_len = sizeof(ix_data)
    };

    sol_invoke(&instruction, infos, params->ka_num);
}

uint64_t helloworld(SolParameters *params) {
sol_assert(params->ka_num == 7);
    sol_assert(params->data_len == 3);

    SolAccountInfo *accnts = params->ka;

    system_create(params);
    withdraw(params, accnts[USER].key, accnts[VAULT].key, 0x280, 50000);

    return SUCCESS;
}

extern uint64_t entrypoint(const uint8_t *input) {
    sol_log("My custom eBPF running");

    SolAccountInfo accounts[7];
    SolParameters params = (SolParameters){.ka = accounts};

    if (!sol_deserialize(input, &params, SOL_ARRAY_SIZE(accounts))) {
    return ERROR_INVALID_ARGUMENT;
    }

    return helloworld(&params);
}
