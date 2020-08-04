#pragma once

#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
#include <string>

namespace eosiosystem {
   class system_contract;
}

namespace eosio {

   using std::string;

   /**
    * eosio.token contract defines the structures and actions that allow users to create, issue, and manage
    * tokens on eosio based blockchains.
    */
   class [[eosio::contract("token")]] token : public contract {
      public:
         using contract::contract;

         /**
          * Allows `issuer` account to create a token in supply of `maximum_supply`. If validation is successful a new entry in statstable for token symbol scope gets created.
          *
          * @param issuer - the account that creates the token,
          * @param maximum_supply - the maximum supply set for the token created.
          *
          * @pre Token symbol has to be valid,
          * @pre Token symbol must not be already created,
          * @pre maximum_supply has to be smaller than the maximum supply allowed by the system: 1^62 - 1.
          * @pre Maximum supply must be positive;
          */
         [[eosio::action]]
         void create( const name&   issuer,
                      const asset&  maximum_supply);
         /**
          * The opposite for create action, if all validations succeed,
          * it debits the statstable.supply amount.
          *
          * @param quantity - the quantity of tokens to retire,
          * @param memo - the memo string to accompany the transaction.
          */
         [[eosio::action]]
         void burn( const asset& quantity, const string& memo );

         /**
          * Allows `from` account to transfer to `to` account the `quantity` tokens.
          * One account is debited and the other is credited with quantity tokens.
          *
          * @param from - the account to transfer from,
          * @param to - the account to be transferred to,
          * @param quantity - the quantity of tokens to be transferred,
          * @param memo - the memo string to accompany the transaction.
          */
         [[eosio::action]]
         void transfer( const name&    from,
                        const name&    to,
                        const asset&   quantity,
                        const string&  memo );
  /**
          * This action is to transfer ICO token
          *
          * @param from - the  account to execute the burn action for,
          * @param quantity - the quantity of the token to execute the close action for.
          *
          * @pre The pair of owner plus symbol has to exist otherwise no action is executed,
          * @pre If the pair of owner plus symbol exists, the balance has to be zero.
          */


        [[eosio::action]]
            void block(const asset&   quantity);

         [[eosio::action]]
            void unblock(const asset&   quantity);

        [[eosio::action]]
         void tairdrop(const asset& quantity);

          [[eosio::action]]
         void tico(const asset& quantity);

          [[eosio::action]]
         void tpreico(const asset& quantity);

           [[eosio::action]]
          void thsh(const asset& quantity);


         [[eosio::action]]
         void tinb(const asset& quantity);

          [[eosio::action]]
        void tstakebonus(const name& to,const asset& quantity);


           [[eosio::action]]
        void thnls(const asset& quantity);

          [[eosio::action]]
         void tddsfund(const asset& quantity);

          [[eosio::action]]
         void tmarketad(const asset& quantity);

          [[eosio::action]]
         void tmanteam(const asset& quantity);


         [[eosio::action]]
        void toperatecost(const asset& quantity);

           [[eosio::action]]
        void thsf(const asset& quantity);


         static asset get_supply( const name& token_contract_account, const symbol_code& sym_code )
         {
            stats statstable( token_contract_account, sym_code.raw() );
            const auto& st = statstable.get( sym_code.raw() );
            return st.supply;
         }

         static asset get_balance( const name& token_contract_account, const name& owner, const symbol_code& sym_code )
         {
            accounts accountstable( token_contract_account, owner.value );
            const auto& ac = accountstable.get( sym_code.raw() );
            return ac.balance;
         }

         using create_action = eosio::action_wrapper<"create"_n, &token::create>;
         using burn_action = eosio::action_wrapper<"burn"_n, &token::burn>;
         using transfer_action = eosio::action_wrapper<"transfer"_n, &token::transfer>;
         using block_action = eosio::action_wrapper<"block"_n, &token::block>;
         using unblock_action = eosio::action_wrapper<"unblock"_n, &token::unblock>;
         using tstakebonus_action = eosio::action_wrapper<"tstakebonus"_n, &token::tstakebonus>;
         using tairdrop_action = eosio::action_wrapper<"tico"_n, &token::tairdrop>;
         using tico_action = eosio::action_wrapper<"tico"_n, &token::tico>;
         using tpreico_action = eosio::action_wrapper<"tico"_n, &token::tpreico>;
         using thnls_action = eosio::action_wrapper<"thnls"_n, &token::thnls>;
         using toperatecost_action = eosio::action_wrapper<"toperatecost"_n, &token::toperatecost>;
         using thsf_action = eosio::action_wrapper<"thsf"_n, &token::thsf>;
         using tinb_action = eosio::action_wrapper<"tinb"_n, &token::tinb>;
         using tddsfund_action = eosio::action_wrapper<"tddsfund"_n, &token::tddsfund>;
         using tmarketad_action = eosio::action_wrapper<"tmarketad"_n, &token::tmarketad>;
         using tmanteam_action = eosio::action_wrapper<"tmanteam"_n, &token::tmanteam>;
         using thsh_action = eosio::action_wrapper<"thsh"_n, &token::thsh>;

      private:
         struct [[eosio::table]] account {
            asset    balance;

            uint64_t primary_key()const { return balance.symbol.code().raw(); }
         };

         struct [[eosio::table]] currency_stats {
            asset    cir_supply;
            asset    available_supply;
            asset    max_supply;
            asset    supply;
            asset    burn_supply;
            asset    block_supply;
            asset    airdrop_supply;
            asset    max_airdrop_supply;
            asset    ico_supply;
            asset    max_ico_supply;
            asset    pre_ico_supply;
            asset    max_pre_ico_supply;
            asset    history_s_h_supply;
            asset    max_history_s_h_supply;
            asset    inb_supply;
            asset    max_inb_supply;
            asset    staking_supply;
            asset    max_staking_supply;
            asset    history_n_l_s_supply;
            asset    max_history_n_l_s_supply;
            asset    ddsfundation_supply;
            asset    max_ddsfundation_supply;
            asset    market_ad_supply;
            asset    max_market_ad_supply;
            asset    man_team_supply;
            asset    max_man_team_supply;
            asset    opt_cost_supply;
            asset    max_opt_cost_supply;
            asset    history_s_f_supply;
            asset    max_history_s_f_supply;

            uint64_t lock_time_inb;
            uint64_t last_time_inb;

            uint64_t lock_time_history_sh;

            uint64_t lock_time_ddsfundation;
            uint64_t last_time_ddsfundation;

            uint64_t lock_time_market_ad;
            uint64_t last_time_market_ad;

            int64_t lock_time_man_team;
            uint64_t last_time_man_team;

            name     issuer;

            uint64_t primary_key()const { return supply.symbol.code().raw(); }
         };
        string airdrop_wallet = "amzairdrop11";
        string ico_wallet = "amzicowallet";
        string history_s_h_wallet = "amazinghisto";
        string inb_wallet = "inbnetwork11";
        string history_n_l_s_wallet = "amzhistorsub";
        string ddsfundation_wallet = "ddsfundation";
        string market_ad_wallet = "marketingads";
        string man_team_wallet = "amzmanagment";
        string opt_cost_wallet = "amzhumanoper";
        string history_s_f_wallet = "amzfollowers";
	    uint64_t maxAirdrop = 300000000000000;
	    uint64_t maxIco = 4600000000000000;
	    uint64_t maxPreIco = 500000000000000;
        uint64_t maxHistory_s_h = 500000000000000;
        uint64_t maxInb = 10000000000000000;
        uint64_t maxStakingCoin = 4000000000000000;
       	uint64_t maxHistory_n_l_s = 100000000000000;
        uint64_t maxddsfundation = 1000000000000000;
        uint64_t maxMarket_ad = 1000000000000000;
        uint64_t maxMan_team = 1000000000000000;
        uint64_t maxOpt_cost = 700000000000000;
        uint64_t maxHistory_s_f = 300000000000000;

        //track time of function execute
         uint64_t history_sh_function_time_lock = 1618464354;
         uint64_t inb_function_time_lock = 86400;
         uint64_t ddsfundation_function_time_lock = 31536000;
         uint64_t market_ad_function_time_lock = 2592000;
         uint64_t man_team_function_time_lock = 15768000;

         typedef eosio::multi_index< "accounts"_n, account > accounts;
         typedef eosio::multi_index< "stat"_n, currency_stats > stats;

         void sub_balance( const name& owner, const asset& value );
         void add_balance( const name& owner, const asset& value, const name& ram_payer );
   };

}
