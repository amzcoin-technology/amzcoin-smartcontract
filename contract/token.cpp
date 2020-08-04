#include "token.hpp"

namespace eosio {

void token::create( const name&   issuer,
                    const asset&  maximum_supply )
{
    require_auth( get_self() );

    auto sym = maximum_supply.symbol;
    check( sym.is_valid(), "invalid symbol name" );
    check( maximum_supply.is_valid(), "invalid supply");
    check( maximum_supply.amount > 0, "max-supply must be positive");
    check( maximum_supply.amount >= (maxAirdrop+maxIco+maxPreIco+maxHistory_s_h+maxInb+maxStakingCoin+maxHistory_n_l_s+maxddsfundation+maxMarket_ad+maxMan_team+maxOpt_cost+maxHistory_s_f), "max-supply must be greater than allocated transfers summation");
    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing == statstable.end(), "token with symbol already exists" );
    statstable.emplace( get_self(), [&]( auto& s ) {
       s.supply.symbol = maximum_supply.symbol;
       s.cir_supply.symbol = maximum_supply.symbol;
       s.burn_supply.symbol = maximum_supply.symbol;
       s.block_supply.symbol    = maximum_supply.symbol;
        s.max_supply    = maximum_supply;
        s.available_supply    = maximum_supply;

        s.max_airdrop_supply.symbol = maximum_supply.symbol;
        s.max_airdrop_supply.amount = maxAirdrop;
        s.airdrop_supply.symbol = maximum_supply.symbol;

        s.max_ico_supply.symbol = maximum_supply.symbol;
        s.max_ico_supply.amount = maxIco;
        s.ico_supply.symbol = maximum_supply.symbol;

        s.max_pre_ico_supply.symbol = maximum_supply.symbol;
        s.max_pre_ico_supply.amount = maxPreIco;
        s.pre_ico_supply.symbol = maximum_supply.symbol;

           s.max_history_s_h_supply.symbol = maximum_supply.symbol;
        s.max_history_s_h_supply.amount = maxHistory_s_h;
        s.history_s_h_supply.symbol = maximum_supply.symbol;


        s.max_inb_supply.symbol = maximum_supply.symbol;
        s.max_inb_supply.amount = maxInb;
        s.inb_supply.symbol = maximum_supply.symbol;

       s.max_staking_supply.symbol = maximum_supply.symbol;
       s.max_staking_supply.amount = maxStakingCoin;
       s.staking_supply.symbol = maximum_supply.symbol;

        s.max_history_n_l_s_supply.symbol = maximum_supply.symbol;
        s.max_history_n_l_s_supply.amount = maxHistory_n_l_s;
        s.history_n_l_s_supply.symbol = maximum_supply.symbol;

        s.max_ddsfundation_supply.symbol = maximum_supply.symbol;
        s.max_ddsfundation_supply.amount = maxddsfundation;
        s.ddsfundation_supply.symbol = maximum_supply.symbol;

        s.max_market_ad_supply.symbol = maximum_supply.symbol;
        s.max_market_ad_supply.amount = maxMarket_ad;
        s.market_ad_supply.symbol = maximum_supply.symbol;


        s.max_man_team_supply.symbol = maximum_supply.symbol;
        s.max_man_team_supply.amount = maxMan_team;
        s.man_team_supply.symbol = maximum_supply.symbol;

        s.max_opt_cost_supply.symbol = maximum_supply.symbol;
        s.max_opt_cost_supply.amount = maxOpt_cost;
        s.opt_cost_supply.symbol = maximum_supply.symbol;

        s.max_history_s_f_supply.symbol = maximum_supply.symbol;
        s.max_history_s_f_supply.amount = maxHistory_s_f;
        s.history_s_f_supply.symbol = maximum_supply.symbol;

       s.lock_time_history_sh = history_sh_function_time_lock;

        s.lock_time_inb = inb_function_time_lock;
       s.last_time_inb = 0;

       s.lock_time_ddsfundation = ddsfundation_function_time_lock;
       s.last_time_ddsfundation=0;

       s.lock_time_market_ad = market_ad_function_time_lock;
       s.last_time_market_ad=0;

       s.lock_time_man_team = man_team_function_time_lock;
       s.last_time_man_team=0;

       s.issuer        = issuer;
    });
}

void token::burn( const asset& quantity, const string& memo )
{
    auto sym = quantity.symbol;
    check( sym.is_valid(), "invalid symbol name" );
    check( memo.size() <= 256, "memo has more than 256 bytes" );

    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing != statstable.end(), "token with symbol does not exist" );
    const auto& st = *existing;

    require_auth( st.issuer );
    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must retire positive quantity" );

    check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    check( quantity.amount <= st.cir_supply.amount , "quantity exceeds available circulating supply");

    statstable.modify( st, same_payer, [&]( auto& s ) {
       s.cir_supply -= quantity;
       s.available_supply -= quantity;
       s.burn_supply += quantity;
    });

    sub_balance( st.issuer, quantity );
}

void token::transfer( const name&    from,
                      const name&    to,
                      const asset&   quantity,
                      const string&  memo )
{
    check( from != to, "cannot transfer to self" );
    require_auth( from );
    check( is_account( to ), "to account does not exist");
    auto sym = quantity.symbol.code();
    stats statstable( get_self(), sym.raw() );
    const auto& st = statstable.get( sym.raw() );

    require_recipient( from );
    require_recipient( to );

    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must transfer positive quantity" );
    check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    check( memo.size() <= 256, "memo has more than 256 bytes" );

    auto payer = has_auth( to ) ? to : from;

    sub_balance( from, quantity );
    add_balance( to, quantity, payer );
}

void token::sub_balance( const name& owner, const asset& value ) {
   accounts from_acnts( get_self(), owner.value );

   const auto& from = from_acnts.get( value.symbol.code().raw(), "no balance object found" );
   check( from.balance.amount >= value.amount, "overdrawn balance" );

   from_acnts.modify( from, owner, [&]( auto& a ) {
         a.balance -= value;
      });
}

void token::add_balance( const name& owner, const asset& value, const name& ram_payer )
{
   accounts to_acnts( get_self(), owner.value );
   auto to = to_acnts.find( value.symbol.code().raw() );
   if( to == to_acnts.end() ) {
      to_acnts.emplace( ram_payer, [&]( auto& a ){
        a.balance = value;
      });
   } else {
      to_acnts.modify( to, same_payer, [&]( auto& a ) {
        a.balance += value;
      });
   }
}

void token::tairdrop(const asset& quantity){
    require_auth( get_self() );
    const name & to = name(airdrop_wallet);
    auto sym = quantity.symbol;
    check( sym.is_valid(), "invalid symbol name" );

    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing != statstable.end(), "token with symbol does not exist, create token before transfer" );
    const auto& st = *existing;
    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must issue positive quantity" );

    check( quantity.symbol == st.max_airdrop_supply.symbol, "symbol precision mismatch" );
    check( st.max_airdrop_supply.amount >= st.airdrop_supply.amount, "All Airdrop Tokens are transferred" );
    check( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");
    check( quantity.amount <= st.max_airdrop_supply.amount - st.airdrop_supply.amount, "quantity exceeds available Airdrop supply");

    statstable.modify( st, same_payer, [&]( auto& s ) {
       s.airdrop_supply += quantity;
       s.supply += quantity;
       s.cir_supply +=quantity;
       });

       add_balance( to, quantity, get_self() );

}

void token::tico(const asset& quantity){
    require_auth( get_self() );
    const name & to = name(ico_wallet);
    auto sym = quantity.symbol;
    check( sym.is_valid(), "invalid symbol name" );

    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing != statstable.end(), "token with symbol does not exist, create token before transfer" );
    const auto& st = *existing;
    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must issue positive quantity" );

    check( quantity.symbol == st.max_ico_supply.symbol, "symbol precision mismatch" );
    check( st.max_ico_supply.amount >= st.ico_supply.amount, "All ICO Tokens are transferred" );
    check( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");
    check( quantity.amount <= st.max_ico_supply.amount - st.ico_supply.amount, "quantity exceeds available ICO supply");

    statstable.modify( st, same_payer, [&]( auto& s ) {
       s.ico_supply += quantity;
       s.supply += quantity;
       s.cir_supply +=quantity;
       });

       add_balance( to, quantity, get_self() );

}

void token::tpreico(const asset& quantity){
    require_auth( get_self() );
    const name & to = name(ico_wallet);
    auto sym = quantity.symbol;
    check( sym.is_valid(), "invalid symbol name" );

    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing != statstable.end(), "token with symbol does not exist, create token before transfer" );
    const auto& st = *existing;
    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must issue positive quantity" );

    check( quantity.symbol == st.max_pre_ico_supply.symbol, "symbol precision mismatch" );
    check( st.max_pre_ico_supply.amount >= st.pre_ico_supply.amount, "All Pre-ICO Tokens are transferred" );
    check( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");
    check( quantity.amount <= st.max_pre_ico_supply.amount - st.pre_ico_supply.amount, "quantity exceeds available Pre-ICO supply");

    statstable.modify( st, same_payer, [&]( auto& s ) {
       s.pre_ico_supply += quantity;
       s.supply += quantity;
       s.cir_supply +=quantity;
       });

       add_balance( to, quantity, get_self() );

}

void token::thsh(const asset& quantity){
    require_auth( get_self() );
    const name & to = name(history_s_h_wallet);
     auto sym = quantity.symbol;
    check( sym.is_valid(), "invalid symbol name" );

    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing != statstable.end(), "token with symbol does not exist, create token before transfer" );
    const auto& st = *existing;
    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must issue positive quantity" );

    check( quantity.symbol == st.max_ico_supply.symbol, "symbol precision mismatch" );
    check( st.max_history_s_h_supply.amount >= st.history_s_h_supply.amount, "All Amazing History ShareHolder Tokens are transferred" );
    check( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");
    check( quantity.amount <= st.max_history_s_h_supply.amount - st.history_s_h_supply.amount, "quantity exceeds available Amazing History ShareHolder supply");
    uint64_t currentTime = current_time_point().sec_since_epoch();
    check( currentTime>=st.lock_time_history_sh, "Lock time not finished for Amazing History ShareHolder");

    //de
    statstable.modify( st, same_payer, [&]( auto& s ) {
       s.history_s_h_supply += quantity;
     s.supply += quantity;
     s.cir_supply +=quantity;
           });

       add_balance( to, quantity, get_self() );

}

 void token::tinb(const asset&   quantity){
    require_auth( get_self() );
    const name & to = name(inb_wallet);
     auto sym = quantity.symbol;
    check( sym.is_valid(), "invalid symbol name" );

    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing != statstable.end(), "token with symbol does not exist, create token before transfer" );
    const auto& st = *existing;
    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must issue positive quantity" );

    check( quantity.symbol == st.inb_supply.symbol, "symbol precision mismatch" );
    check( st.max_inb_supply.amount > st.inb_supply.amount, "All INB Tokens are transferred" );
    check( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");
    check( quantity.amount <= st.max_inb_supply.amount - st.inb_supply.amount, "quantity exceeds available INB supply");
    uint64_t currentTime = current_time_point().sec_since_epoch();
    check( currentTime-st.last_time_inb>=st.lock_time_inb, "Lock time not finished for INB");

    //decrease supply
    statstable.modify( st, same_payer, [&]( auto& s ) {
       s.inb_supply +=quantity;
       s.last_time_inb = currentTime;
    s.supply += quantity;
          s.cir_supply +=quantity;
          });
       add_balance( to, quantity, get_self() );
}

void token::tstakebonus(const name& to,const asset& quantity){
    require_auth( get_self() );
    auto sym = quantity.symbol;
    check( sym.is_valid(), "invalid symbol name" );

    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing != statstable.end(), "token with symbol does not exist, create token before issue" );
    const auto& st = *existing;
    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must issue positive quantity" );


    check( quantity.symbol == st.staking_supply.symbol, "symbol precision mismatch" );
    check( st.max_staking_supply.amount >= st.staking_supply.amount, "All Staking Tokens are transferred" );
        check( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");
    check( quantity.amount <= st.max_staking_supply.amount - st.staking_supply.amount, "quantity exceeds available staking supply");

    statstable.modify( st, same_payer, [&]( auto& s ) {
       s.staking_supply += quantity;
  s.supply += quantity;
        s.cir_supply +=quantity;
        });

       add_balance( to, quantity, get_self() );

}


void token::thnls(const asset& quantity){
    require_auth( get_self() );
    const name & to = name(history_n_l_s_wallet);
     auto sym = quantity.symbol;
    check( sym.is_valid(), "invalid symbol name" );

    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing != statstable.end(), "token with symbol does not exist, create token before transfer" );
    const auto& st = *existing;
    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must issue positive quantity" );

    check( quantity.symbol == st.max_history_n_l_s_supply.symbol, "symbol precision mismatch" );
    check( st.max_history_n_l_s_supply.amount >= st.history_n_l_s_supply.amount, "All History NewsLetter Subscribers Tokens are transferred" );
        check( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");
    check( quantity.amount <= st.max_history_n_l_s_supply.amount - st.history_n_l_s_supply.amount, "quantity exceeds available History NewsLetter Subscribers Tokens supply");

    statstable.modify( st, same_payer, [&]( auto& s ) {
       s.history_n_l_s_supply += quantity;
    s.supply += quantity;
          s.cir_supply +=quantity;
          });
       add_balance( to, quantity, get_self() );


}

 void token::tddsfund(const asset&   quantity){
      require_auth( get_self() );
      const name & to = name(ddsfundation_wallet);
     auto sym = quantity.symbol;
    check( sym.is_valid(), "invalid symbol name" );

    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing != statstable.end(), "token with symbol does not exist, create token before transfer" );
    const auto& st = *existing;
    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must issue positive quantity" );

    check( quantity.symbol == st.ddsfundation_supply.symbol, "symbol precision mismatch" );
    check( st.max_ddsfundation_supply.amount > st.ddsfundation_supply.amount, "All Philanthropy Tokens are transferred" );
        check( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");
    check( quantity.amount <= st.max_ddsfundation_supply.amount - st.ddsfundation_supply.amount, "quantity exceeds available Philanthropy Tokens supply");
    uint64_t currentTime = current_time_point().sec_since_epoch();
    check( currentTime-st.last_time_ddsfundation>=st.lock_time_ddsfundation, "Lock time not finished for Philanthropy");

    //decrease supply
    statstable.modify( st, same_payer, [&]( auto& s ) {
       s.ddsfundation_supply +=quantity;
       s.last_time_ddsfundation = currentTime;
    s.supply += quantity;
          s.cir_supply +=quantity;
          });
        add_balance( to, quantity, get_self() );
}

 void token::tmarketad(const asset&   quantity){
 require_auth( get_self() );
   const name & to = name(market_ad_wallet);
     auto sym = quantity.symbol;
    check( sym.is_valid(), "invalid symbol name" );

    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing != statstable.end(), "token with symbol does not exist, create token before transfer" );
    const auto& st = *existing;
    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must issue positive quantity" );

    check( quantity.symbol == st.market_ad_supply.symbol, "symbol precision mismatch" );
    check( st.max_market_ad_supply.amount > st.market_ad_supply.amount, "All Marketing Ads Tokens are transferred" );
        check( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");
    check( quantity.amount <= st.max_market_ad_supply.amount - st.market_ad_supply.amount, "quantity exceeds available Marketing Ads Tokens supply");
    uint64_t currentTime = current_time_point().sec_since_epoch();
    check( currentTime-st.last_time_market_ad>=st.lock_time_market_ad, "Lock time not finished for Marketing Ads Tokens");

    //decrease supply
    statstable.modify( st, same_payer, [&]( auto& s ) {
       s.market_ad_supply +=quantity;
       s.last_time_market_ad = currentTime;
    s.supply += quantity;
          s.cir_supply +=quantity;
          });
        add_balance( to, quantity, get_self() );
}

 void token::tmanteam(const asset&   quantity){
 require_auth( get_self() );
   const name & to = name(man_team_wallet);
     auto sym = quantity.symbol;
    check( sym.is_valid(), "invalid symbol name" );

    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing != statstable.end(), "token with symbol does not exist, create token before transfer" );
    const auto& st = *existing;
    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must issue positive quantity" );

    check( quantity.symbol == st.man_team_supply.symbol, "symbol precision mismatch" );
    check( st.max_man_team_supply.amount > st.man_team_supply.amount, "All Management Team Tokens are transferred" );
        check( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");
    check( quantity.amount <= st.max_man_team_supply.amount - st.man_team_supply.amount, "quantity exceeds available Management Team Tokens supply");
    uint64_t currentTime = current_time_point().sec_since_epoch();
    check( currentTime-st.last_time_man_team>=st.lock_time_man_team, "Lock time not finished for Management Team Tokens");

    //decrease supply
    statstable.modify( st, same_payer, [&]( auto& s ) {
       s.man_team_supply +=quantity;
       s.last_time_man_team = currentTime;
    s.supply += quantity;
          s.cir_supply +=quantity;
          });
       add_balance( to, quantity, get_self() );
}

void token::toperatecost(const asset& quantity){
require_auth( get_self() );
   const name & to = name(opt_cost_wallet);
     auto sym = quantity.symbol;
    check( sym.is_valid(), "invalid symbol name" );

    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing != statstable.end(), "token with symbol does not exist, create token before transfer" );
    const auto& st = *existing;
    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must issue positive quantity" );


    check( quantity.symbol == st.max_opt_cost_supply.symbol, "symbol precision mismatch" );
    check( st.max_opt_cost_supply.amount >= st.opt_cost_supply.amount, "All Operating Cost Tokens are transferred" );
        check( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");
    check( quantity.amount <= st.max_opt_cost_supply.amount - st.opt_cost_supply.amount, "quantity exceeds available Operating Cost Tokens supply");

    statstable.modify( st, same_payer, [&]( auto& s ) {
       s.opt_cost_supply += quantity;
   s.supply += quantity;
         s.cir_supply +=quantity;
         });
      add_balance( to, quantity, get_self() );

}

void token::thsf(const asset& quantity){
require_auth( get_self() );
    const name & to = name(history_s_f_wallet);
     auto sym = quantity.symbol;
    check( sym.is_valid(), "invalid symbol name" );

    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing != statstable.end(), "token with symbol does not exist, create token before transfer" );
    const auto& st = *existing;
    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must issue positive quantity" );


    check( quantity.symbol == st.max_history_s_f_supply.symbol, "symbol precision mismatch" );
    check( st.max_history_s_f_supply.amount >= st.history_s_f_supply.amount, "All Amazing Subscribers and Followers Tokens are transferred" );
        check( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");
    check( quantity.amount <= st.max_history_s_f_supply.amount - st.history_s_f_supply.amount, "quantity exceeds available Subscribers and Followers Tokens supply");

    statstable.modify( st, same_payer, [&]( auto& s ) {
       s.history_s_f_supply += quantity;
    s.supply += quantity;
          s.cir_supply +=quantity;
          });

     add_balance( to, quantity, get_self() );
}

void token::block(const asset&   quantity)
{

    auto sym = quantity.symbol;
    check( sym.is_valid(), "invalid symbol name" );
    check( quantity.is_valid(), "invalid supply");
    check( quantity.amount > 0, "amount must be positive");
//find token from stats table
    stats statstable( get_self(), sym.code().raw() );
    auto existingstat = statstable.find( sym.code().raw() );
    check( existingstat != statstable.end(), "token with symbol does not exist, create token before blocking it" );
    const auto& st = *existingstat;
    require_auth( st.issuer );
    check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    check( quantity.amount <= st.cir_supply.amount, "quantity exceeds available circulating supply");
//decrease supply
    statstable.modify( st, same_payer, [&]( auto& s ) {
       s.cir_supply -= quantity;
       s.block_supply += quantity;
    });
    sub_balance( st.issuer, quantity );
}
void token::unblock(const asset& quantity)
{
    auto sym = quantity.symbol;
    check( sym.is_valid(), "invalid symbol name" );
    check( quantity.is_valid(), "invalid supply");
    check( quantity.amount > 0, "amount must be positive");
//find token from stats table
    stats statstable( get_self(), sym.code().raw() );
    auto existingstat = statstable.find( sym.code().raw() );
    check( existingstat != statstable.end(), "token with symbol does not exist, create and block the token before un-blocking it" );
    const auto& st = *existingstat;
    require_auth( st.issuer );
    check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    check( quantity.amount <= st.block_supply.amount, "quantity exceeds available blocked token");
   //decrease supply
    statstable.modify( st, same_payer, [&]( auto& s ) {
       s.cir_supply += quantity;
       s.block_supply -= quantity;
    });
      add_balance( st.issuer, quantity, st.issuer );
}
}/// namespace eosio
