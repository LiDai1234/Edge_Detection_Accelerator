library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity ov7670_sobel_top is
    Generic (
        IMG_WIDTH  : integer := 160;   -- 一行像素数
        IMG_HEIGHT : integer := 146;   -- 一帧行数
        THRESHOLD  : integer := 120    -- Sobel 阈值
    );
    Port (
        -- 系统时钟与复位
        clk         : in  std_logic;
        reset_n     : in  std_logic;

        -- OV7670 FIFO 接口
        fifo_d      : in  std_logic_vector(7 downto 0);  -- 8 位数据输入
        fifo_rck    : out std_logic;                     -- 读时钟，由 FPGA 产生
        fifo_rrst   : out std_logic;                     -- 读指针复位（帧开始时使用）

        -- STM32 接口
        pixel_out   : out std_logic_vector(15 downto 0); -- RGB565 像素
        pixel_valid : out std_logic                      -- 接 STM32 PC8，高电平表示数据有效
    );
end ov7670_sobel_top;

architecture Behavioral of ov7670_sobel_top is

    -- 状态机定义
    type state_t is (
        IDLE,           -- 等待帧开始
        RESET_FIFO,     -- 复位 FIFO 读指针
        READ_BYTE1,     -- 读像素高字节
        READ_BYTE2,     -- 读像素低字节
        WAIT_PROC,      -- 等待流水线处理
        OUTPUT_PIXEL,   -- 输出像素并拉高 pixel_valid
        WAIT_ACK,       -- 等待 STM32 读取
        NEXT_PIXEL      -- 准备下一个像素
    );
    signal state : state_t := IDLE;

    -- 像素坐标计数器
    signal x_cnt : integer range 0 to IMG_WIDTH  := 0;
    signal y_cnt : integer range 0 to IMG_HEIGHT := 0;

    -- FIFO 读控制
    signal rck_int : std_logic := '0';
    signal byte_high : std_logic_vector(7 downto 0) := (others => '0');

    -- 行缓冲
    type line_buf_t is array (0 to IMG_WIDTH-1) of unsigned(7 downto 0);
    signal line_buf0 : line_buf_t := (others => (others => '0'));
    signal line_buf1 : line_buf_t := (others => (others => '0'));
    signal line_sel  : std_logic := '0';  -- 0: 当前写 line_buf0，1: line_buf1

    -- 3x3 窗口寄存器
    signal p00, p01, p02 : unsigned(7 downto 0) := (others => '0');
    signal p10, p11, p12 : unsigned(7 downto 0) := (others => '0');
    signal p20, p21, p22 : unsigned(7 downto 0) := (others => '0');

    -- Sobel 计算结果
    signal gx, gy : integer := 0;
    signal magnitude : integer := 0;
    signal edge_pixel : std_logic_vector(15 downto 0) := (others => '0');

    -- 简单的延时计数器，用于保持 pixel_valid 一段时间
    signal delay_cnt : integer range 0 to 255 := 0;
    constant DELAY_MAX : integer := 100;  

begin

    -- FIFO 读时钟输出
    fifo_rck <= rck_int;

    -- 主状态机
    process(clk, reset_n)
    begin
        if reset_n = '0' then
            state <= IDLE;
            x_cnt <= 0;
            y_cnt <= 0;
            rck_int <= '0';
            fifo_rrst <= '1';  -- 复位低有效，空闲时保持高
            pixel_valid <= '0';
            pixel_out <= (others => '0');
            delay_cnt <= 0;
            line_sel <= '0';
        elsif rising_edge(clk) then

            case state is

                when IDLE =>
                    -- 等待帧同步信号
                    state <= RESET_FIFO;

                when RESET_FIFO =>
                    fifo_rrst <= '0';   -- 拉低复位
                    if delay_cnt = 10 then  -- 保持 10 个时钟
                        fifo_rrst <= '1';
                        delay_cnt <= 0;
                        x_cnt <= 0;
                        y_cnt <= 0;
                        line_sel <= '0';
                        state <= READ_BYTE1;
                    else
                        delay_cnt <= delay_cnt + 1;
                    end if;

                when READ_BYTE1 =>
                    rck_int <= '0';     -- RCK 低电平
                    if delay_cnt = 2 then
                        byte_high <= fifo_d;  -- 在低电平期间读取高字节
                        delay_cnt <= 0;
                        state <= READ_BYTE2;
                    else
                        delay_cnt <= delay_cnt + 1;
                    end if;

                when READ_BYTE2 =>
                    rck_int <= '1';     -- RCK 上升沿，FIFO 输出低字节
                    if delay_cnt = 2 then
                        -- 合并为 RGB565 像素
                        state <= WAIT_PROC;
                        delay_cnt <= 0;
                    else
                        delay_cnt <= delay_cnt + 1;
                    end if;

                when WAIT_PROC =>
                    state <= OUTPUT_PIXEL;

                when OUTPUT_PIXEL =>
                    -- 将边缘检测结果输出到 pixel_out，并拉高 pixel_valid
                    pixel_out <= edge_pixel;  -- edge_pixel 是 Sobel 计算后的二值化 RGB565
                    pixel_valid <= '1';
                    delay_cnt <= 0;
                    state <= WAIT_ACK;

                when WAIT_ACK =>
                    -- 保持 pixel_valid 高电平一段时间，让 STM32 读取
                    if delay_cnt = DELAY_MAX then
                        pixel_valid <= '0';
                        delay_cnt <= 0;
                        -- 更新坐标
                        if x_cnt = IMG_WIDTH - 1 then
                            x_cnt <= 0;
                            if y_cnt = IMG_HEIGHT - 1 then
                                y_cnt <= 0;
                                state <= IDLE;  -- 一帧结束，重新开始
                            else
                                y_cnt <= y_cnt + 1;
                                state <= NEXT_PIXEL;
                            end if;
                        else
                            x_cnt <= x_cnt + 1;
                            state <= NEXT_PIXEL;
                        end if;
                    else
                        delay_cnt <= delay_cnt + 1;
                    end if;

                when NEXT_PIXEL =>
                    state <= READ_BYTE1;

                when others =>
                    state <= IDLE;

            end case;
        end if;
    end process;

    -- 组合逻辑：灰度转换 + 行缓冲写入 + Sobel 计算

end Behavioral;
